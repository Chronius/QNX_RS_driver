/* R E S O U R C E   M A N A G E R S  - THE "NULL DEVICE" */

/* Project Name: "example_resmgr_devnull" */

/* What is a Resource Manager under QNX Neutrino?
 * A resource manager is a superset of a device driver. The QNX
 * resource manager framework is used to create the POSIX
 * interface (open, read, write, etc.) for any resource you
 * can think of. Imagine you are coding a device driver for
 * a device that reads credit cards. Your application would then
 * be able to just use open() to access the card, read() to retrieve
 * data from the card, and write() to store data on the card.
 * If later your hardware changes, and you don't need to read
 * credit cards any more but are reading serial data from some
 * field bus instead, you replace/update the resource manager
 * while your application still just uses open(), read() and write()!
 */

/*
 * This example contains a ready-to-run resource manager. It
 * represents the "null device": Everything works - you can
 * read from it and get zero bytes. And you can write to it,
 * and the write succeeds - the data is written into nowhere.
 * (Note that our device is called /dev/Null instead of
 * /dev/null; this is because /dev/null already exists and is
 * part of Neutrino).
 *
 * Start it as:
 *    example_resmgr_devnull -v &
 * If your target is running Photon, open a separate terminal
 * window for the following tests. Else all will happen on
 * your console.
 * Test the new device by trying:
 *   cat /dev/Null
 * (You will read from the device, and cat will
 * return at once because 0 bytes are returned from this resmgr).
 *
 *   echo hallo > /dev/Null
 * (You will write to the device; it
 * will work and the string will be printed on the resmgr side).
 * Sending whole files into this null device will also work;
 * to make this work, the more complex code in the write handler
 * is actually needed.
 *
 */

/* More information:
 * Please read the Writing a Resource Manager guide or the
 * Resource Managers chapter in Getting Started with QNX Neutrino.
 */


#include <InterruptHandler.h>
#include <signal.h>

extern volatile p_uart_reg p_uart[UART_CHANNEL_COUNT];
extern channel_uart channel[UART_CHANNEL_COUNT];

extern pthread_spinlock_t  fifo_spinlock[UART_CHANNEL_COUNT];
extern void handler();

void options (int argc, char **argv);

/* A resource manager mainly consists of callbacks for POSIX
 * functions a client could call. In the example, we have
 * callbacks for the open(), read() and write() calls. More are
 * possible. If we don't supply own functions (e.g. for stat(),
 * seek(), etc.), the resource manager framework will use default
 * system functions, which in most cases return with an error
 * code to indicate that this resource manager doesn't support
 * this function.*/

/* These prototypes are needed since we are using their names
 * in main(). */

int io_open (resmgr_context_t *ctp, io_open_t  *msg, RESMGR_HANDLE_T *handle, void *extra);
int io_read (resmgr_context_t *ctp, io_read_t  *msg, RESMGR_OCB_T *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);

int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);

/*
 * Our connect and I/O functions - we supply two tables
 * which will be filled with pointers to callback functions
 * for each POSIX function. The connect functions are all
 * functions that take a path, e.g. open(), while the I/O
 * functions are those functions that are used with a file
 * descriptor (fd), e.g. read().
 */

resmgr_connect_funcs_t  connect_funcs;
resmgr_io_funcs_t       io_funcs;

/*
 * Our dispatch, resource manager, and iofunc variables
 * are declared here. These are some small administrative things
 * for our resource manager.
 */

dispatch_t              *dpp;
resmgr_attr_t           rattr;
dispatch_context_t      *ctp;
iofunc_attr_t           ioattr;

char    *progname = "dbg_rs";
int     optv;                               // -v for verbose operation

int main (int argc, char **argv)
{
	int status;
	status = ThreadCtl(_NTO_TCTL_IO, 0);

	if (status == -1)
	{
		printf("XXXXXX: Can't gain i/o access permissions");
		exit(EXIT_FAILURE);
	}

	printf ("%s:  starting...\n", progname);

	/* Check for command line options (-v) */
	options (argc, argv);

	/* Allocate and initialize a dispatch structure for use
	 * by our main loop. This is for the resource manager
	 * framework to use. It will receive messages for us,
	 * analyze the message type integer and call the matching
	 * handler callback function (i.e. io_open, io_read, etc.) */
	dpp = dispatch_create ();
	if (dpp == NULL) {
		fprintf (stderr, "%s:  couldn't dispatch_create: %s\n",
				progname, strerror (errno));
		exit (1);
	}

	/* Set up the resource manager attributes structure. We'll
	 * use this as a way of passing information to
	 * resmgr_attach(). The attributes are used to specify
	 * the maximum message length to be received at once,
	 * and the number of message fragments (iov's) that
	 * are possible for the reply.
	 * For now, we'll just use defaults by setting the
	 * attribute structure to zeroes. */
	memset (&rattr, 0, sizeof (rattr));

	/* Now, let's intialize the tables of connect functions and
	 * I/O functions to their defaults (system fallback
	 * routines) and then override the defaults with the
	 * functions that we are providing. */
	iofunc_func_init (_RESMGR_CONNECT_NFUNCS, &connect_funcs,
			_RESMGR_IO_NFUNCS, &io_funcs);
	/* Now we override the default function pointers with
	 * some of our own coded functions: */
	connect_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	io_funcs.devctl=io_devctl;

	/* Next we call resmgr_attach() to register our device name
	 * with the process manager, and also to let it know about
	 * our connect and I/O functions. */

	char *p;
	char p_buf[2];
	int pathID;

	for (int i = 0; i < UART_CHANNEL_COUNT; i++)
	{
		char name[12] = "/dev/rts/RS";
		iofunc_attr_init (&sample_attrs[i], S_IFNAM | 0666, NULL, NULL);
		p = itoa(i, p_buf, 10);
		strcat(name, p);
		pathID = resmgr_attach (dpp, &rattr, name,
					_FTYPE_ANY, 0, &connect_funcs, &io_funcs, &sample_attrs[i]);
		printf("%s  %d\n", name, pathID);
	}

	pthread_create (NULL, NULL, interrupt_thread, NULL);
	pthread_create (NULL, NULL, handler, NULL);
	/* Now we allocate some memory for the dispatch context
	 * structure, which will later be used when we receive
	 * messages. */
	ctp = dispatch_context_alloc (dpp);

	/* Done! We can now go into our "receive loop" and wait
	 * for messages. The dispatch_block() function is calling
	 * MsgReceive() under the covers, and receives for us.
	 * The dispatch_handler() function analyzes the message
	 * for us and calls the appropriate callback function. */
	while (1) {
		if ((ctp = dispatch_block (ctp)) == NULL) {
			fprintf (stderr, "%s:  dispatch_block failed: %s\n",
					progname, strerror (errno));
			exit (1);
		}
		/* Call the correct callback function for the message
		 * received. This is a single-threaded resource manager,
		 * so the next request will be handled only when this
		 * call returns. Consult our documentation if you want
		 * to create a multi-threaded resource manager. */
		dispatch_handler (ctp);
	}
}

/*
 *  io_open
 *
 * We are called here when the client does an open().
 * In this simple example, we just call the default routine
 * (which would be called anyway if we did not supply our own
 * callback), which creates an OCB (Open Context Block) for us.
 * In more complex resource managers, you will want to check if
 * the hardware is available, for example.
 */

	int
io_open (resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
	if (optv) {
		printf ("%s:  in io_open\n", progname);
	}

	return (iofunc_open_default (ctp, msg, handle, extra));
}

/*
 *  io_read
 *
 * The message that we received can be accessed via the
 * pointer *msg. A pointer to the OCB that belongs to this
 * read is the *ocb. The *ctp pointer points to a context
 * structure that is used by the resource manager framework
 * to determine whom to reply to, and more. */

int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
	size_t nbytes = 0;
	int status;
	int nonblock;
	/* Here we verify if the client has the access
	 * rights needed to read from our device */
	if ((status = iofunc_read_verify(ctp, msg, ocb, &nonblock)) != EOK)
	{
		return (status);
	}
	/* Client does`n want blocking */
	if (nonblock)
	{
		return (EAGAIN);
	}
	/* We check if our read callback was called because of
	 * a pread() or a normal read() call. If pread(), we return
	 * with an error code indicating that we don't support it.*/
	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
	{
		return (ENOSYS);
	}
	/*	ocb->attr указывает на sample_attrs[i].nbytes
	 *  в контексте данной сессии
	 */


/*
	sigset_t *set;
	siginfo_t info;
	SignalWaitinfo(set, &info);
*/

	unsigned char *buffer = NULL;
	buffer = malloc(ocb->attr->nbytes + 1);
	if (buffer == NULL)
		return (ENOMEM);
	ocb->attr->nbytes = fifo_count(&channel[ctp->id].rx_fifo);
	nbytes = min(_IO_READ_GET_NBYTES(msg), ocb->attr->nbytes);

//	SignalAction(ctp->rcvid, ,SIGINT, NULL, NULL);
	/*
	 *	Test function Прерывание cat по Ctrl-C ставиться в очередь
	 *	и срабатывает только по прибытию нового символа,
	 *	MsgReply - разблокирует клиента только если буфер не пустой
	 *	Прерывание по Ctrl-D
	 */
	unsigned char *buf = NULL;
	while (true)
	{
		nbytes = fifo_count(&channel[ctp->id].rx_fifo);
		if (nbytes > 0)
		{
			buf = malloc(nbytes + 1);
			if (buf == NULL)
				return (ENOMEM);

			pthread_spin_lock(&fifo_spinlock[ctp->id]);
			fifo_get(&channel[ctp->id].rx_fifo, buf, 0, nbytes);
			MsgReply(ctp->rcvid, nbytes, buf, nbytes);
			pthread_spin_unlock(&fifo_spinlock[ctp->id]);

			ocb->attr->flags |=
			IOFUNC_ATTR_ATIME | IOFUNC_ATTR_DIRTY_TIME;

			free(buf);
			return _RESMGR_NOREPLY ;
		}
	}
	/*
	 *	Cat реагирует на сигнал Ctrl+C, но при
	 *	выводе в файл терминальные нули '\0'
	 *	также будут записаны
	 */
	if (nbytes > 0)
	{
		pthread_spin_lock(&fifo_spinlock[ctp->id]);
		fifo_get(&channel[ctp->id].rx_fifo, buffer, 0, nbytes);
		pthread_spin_unlock(&fifo_spinlock[ctp->id]);

		/* set up the return data IOV */
		MsgReply(ctp->rcvid, nbytes, buffer, nbytes);
		ocb->attr->flags |=
		IOFUNC_ATTR_ATIME | IOFUNC_ATTR_DIRTY_TIME;
		/*
		 * advance the offset by the number of bytes
		 * returned to the client.
		 */
		ocb->offset += nbytes;
	}
	else
	{
//		nparts = 0;
		char str = '\0';
		MsgReply(ctp->rcvid, 1, &str, 1);
	}
	/* mark the access time as invalid (we just accessed it) */

	/* The next line (commented) is used to tell the system how
	 * large your buffer is in which you want to return your
	 * data for the read() call.
	 */
	if (nbytes > 0)
	{
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;
	}
	/* We return 0 parts, because we are the null device.
	 * Normally, if you return actual data, you would return at
	 * least 1 part. A pointer to and a buffer length for 1 part
	 * are located in the ctp structure.  */
	free(buffer);
	return _RESMGR_NOREPLY;
}

/*
 *  io_write
 *
 *  At this point, the client has called the library write()
 *  function, and expects that our resource manager will write
 *  the number of bytes that have been specified to the device.
 */

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
	int status;
	char *buf;

	/* Check the access permissions of the client */
	if ((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK) {
		return (status);
	}

	/* Check if pwrite() or normal write() */
	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
		return (ENOSYS);
	}

	/* First check if our message buffer was large enough
	 * to receive the whole write at once.
	 */
	if ((msg->i.nbytes <= ctp->info.msglen - ctp->offset - sizeof(msg->i))
			&& (ctp->info.msglen < ctp->msg_max_size))
	{ 
		/* space for NULL byte */
		buf = (char *)malloc(127 + 1);
		if (buf == NULL)
			return (ENOMEM);

		buf[msg->i.nbytes] = '\0';
		int DataSize = 128;

		char byte;
		if (msg->i.nbytes != 0)
		{
			DataSize = (DataSize >= msg->i.nbytes ? msg->i.nbytes : DataSize);
			resmgr_msgread(ctp, buf, DataSize, sizeof(msg->i));

			pthread_spin_lock(&fifo_spinlock[ctp->id]);

			fifo_put(&channel[ctp->id].tx_fifo, buf, 0, DataSize);

			for (int i = 0; i < DataSize; i++)
			{
				fifo_get(&channel[ctp->id].tx_fifo, &byte, 0, 1);
				p_uart[ctp->id]->rbr_thr_dll = byte;
			}
			p_uart[ctp->id]->ier_dlh = p_uart[ctp->id]->ier_dlh | IER_TxD_Set;

			pthread_spin_unlock(&fifo_spinlock[ctp->id]);

			_IO_SET_WRITE_NBYTES (ctp, DataSize);
			ocb->offset += DataSize;
		}
		else
			/* Set the number of bytes successfully written for
			 * the client. This information will be passed to the
			 * client by the resource manager framework upon reply.
			 * In this example, we just take the number of  bytes that
			 * were sent to us and claim we successfully wrote them.
			 */
			_IO_SET_WRITE_NBYTES (ctp, msg -> i.nbytes);
	}
	else
	{
		/* If we did not receive the whole message because the
		 * client wanted to send more than we could receive, we
		 * allocate memory for all the data and use resmgr_msgread()
		 * to read all the data at once. Although we did not receive
		 * the data completely first, because our buffer was not big
		 * enough, the data is still fully available on the client
		 * side, because its write() call blocks until we return
		 * from this callback! */
		buf = malloc(msg->i.nbytes + 1);
		resmgr_msgread(ctp, buf, msg->i.nbytes, sizeof(msg->i));
		buf[msg->i.nbytes] = '\0';
		free(buf);
	}

	/* Finally, if we received more than 0 bytes, we mark the
	 * file information for the device to be updated:
	 * modification time and change of file status time. To
	 * avoid constant update of the real file status information
	 * (which would involve overhead getting the current time), we
	 * just set these flags. The actual update is done upon
	 * closing, which is valid according to POSIX. */
	if (msg->i.nbytes > 0) {
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
	}

	return (_RESMGR_NPARTS (0));
}

int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb)
{
	int nbytes, status;

	struct termios *options_i;
	struct termios *options_o;

	if ((status = iofunc_devctl_default(ctp, msg, ocb)) != _RESMGR_DEFAULT)
	{
		return (status);
	}
	status = nbytes = 0;

	options_i = (struct termios *)_DEVCTL_DATA(msg->i);
	options_o = (struct termios *)_DEVCTL_DATA(msg->o);

	switch (msg->i.dcmd) {
	case DCMD_CHR_TCSETATTR:
		if (CSTOPB & options_i->c_cflag)
			channel[ctp->id].config.stop_bit = 2;
		else
			channel[ctp->id].config.stop_bit = 1;

		channel[ctp->id].config.baud = options_i->c_ispeed;
		nbytes = 0;
		break;

	case DCMD_CHR_TCGETATTR:
		cfsetospeed(options_o, channel[ctp->id].config.baud);
		options_o->c_cflag |= channel[ctp->id].config.data_bits;
		memset(&msg->o, 0, sizeof(msg->o));
		nbytes = sizeof(struct termios);
		break;

	case DCMD_CHR_TTYINFO:
		break;

	default:
		return (ENOSYS);
	}

	// if success
	msg->o.ret_val = status;
	msg->o.nbytes = nbytes;

//	from_config(ctp->id); //setting channel

	return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + nbytes));
}

void handler()
  {

	/*
	 *	инициализация структуры для
	 *	потока обработки сигналов
	 */
	memset(&event, 0, sizeof(event));
	event.sigev_notify = SIGEV_SIGNAL;
	SIGEV_SIGNAL_INIT(&event, SIGINT);
	signal( SIGINT, handler );


//	struct sigaction act;
//	sigset_t set;
//
//	sigemptyset( &set );
//	sigaddset( &set, SIGINT );
//	act.sa_flags = 0;
//	act.sa_mask = set;
//	act.sa_handler = &handler;
//	sigaction( SIGINT, &act, NULL );

	int sig, res;
	sigset_t *set;
	siginfo_t info;
	sigemptyset( &set );
	sigaddset( &set, SIGINT );
	res = sigwaitinfo(&set, &info);
	printf("Received signal %d", info.si_signo);
//	while (true)
//	{
//		res = sigwait(&set, &sig);
//	}
	res = 1;
  }

/*
 *  options
 *
 *  This routine handles the command-line options.
 *  For our simple /dev/Null, we support:
 *      -v      verbose operation
 */

	void
options (int argc, char **argv)
{
	int     opt;

	optv = 0;
	while (optind < argc) {
		while ((opt = getopt (argc, argv, "v")) != -1) {
			switch (opt) {
				case 'v':
					optv = 1;
					break;
			}
		}
	}
}

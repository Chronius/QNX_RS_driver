/* R E S O U R C E   M A N A G E R S  - THE "PCI RS DRIVER */

/* Project Name: "PCI RS driver" */
/*
 * main.c
 *
 *      Author: abuzarov_bv
 */

#include <proto.h>
#include <signal.h>

struct sigevent event;

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

static volatile unsigned done = 0;

static void rs_exit(int signo)
{
	atomic_set(&done, 1);
	dispatch_unblock(ctp);
}

int main (int argc, char **argv)
{
	int status;
	status = ThreadCtl(_NTO_TCTL_IO, 0);

	if (status == -1)
	{
		printf("XXXXXX: Can't gain i/o access permissions");
		exit(EXIT_FAILURE);
	}

	struct sigaction    sa;
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
//	pthread_create (NULL, NULL, handler, NULL);
	/* Now we allocate some memory for the dispatch context
	 * structure, which will later be used when we receive
	 * messages. */
	ctp = dispatch_context_alloc (dpp);


    /* register exit handler */
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);
    sa.sa_handler = rs_exit;
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);

    /* background the process */
    procmgr_daemon(0, PROCMGR_DAEMON_NOCLOSE|PROCMGR_DAEMON_NODEVNULL);

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

void handler()
{	/*
	 *	инициализация структуры для
	 *	потока обработки сигналов
	 */
	memset(&event, 0, sizeof(event));
	event.sigev_notify = SIGEV_SIGNAL;
	SIGEV_SIGNAL_INIT(&event, SIGINT);
	signal( SIGINT, handler);
//	struct sigaction act;
//
//	act.sa_flags = 0;
//	act.sa_mask = set;
//	act.sa_handler = &handler;
//	sigaction( SIGINT, &act, NULL );

	int res;
	sigset_t set;
	siginfo_t info;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	res = sigwaitinfo(&set, &info);
	printf("Received signal %d", info.si_signo);
//	while (true)
//	{
//		res = sigwait(&set, &sig);
//	}

	TM_PRINTF("res", res);
}

/*
 *  options
 *
 *  This routine handles the command-line options.
 *  For our simple /dev/Null, we support:
 *      -v      verbose operation
 */

void options(int argc, char **argv)
{
	int opt;

	optv = 0;
	while (optind < argc)
	{
		while ((opt = getopt(argc, argv, "v")) != -1)
		{
			switch (opt)
			{
			case 'v':
				optv = 1;
				break;
			}
		}
	}
}

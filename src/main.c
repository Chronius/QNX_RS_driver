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

static int pathID[UART_CHANNEL_COUNT];


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
    io_funcs.close_ocb = io_close;
    io_funcs.read = io_read;
    io_funcs.write = io_write;

    io_funcs.devctl = io_devctl;
    io_funcs.unblock = io_unblock;


    /* Next we call resmgr_attach() to register our device name
     * with the process manager, and also to let it know about
     * our connect and I/O functions. */

    char *p;
    char p_buf[2];

    for (int i = 0; i < UART_CHANNEL_COUNT; i++)
    {
        char name[12] = "/dev/rts/RS";
        iofunc_attr_init (&sample_attrs[i], S_IFNAM | 0666, NULL, NULL);
        p = itoa(i, p_buf, 10);
        strcat(name, p);
        pathID[i] = resmgr_attach (dpp, &rattr, name,
                    _FTYPE_ANY, 0, &connect_funcs, &io_funcs, &sample_attrs[i]);
//        printf("%s  %d\n", name, pathID[i]);
    }

    pthread_create (NULL, NULL, interrupt_thread, NULL);

//    pthread_create (NULL, NULL, wait_queue_request, (void*)&p_callback);
#ifdef POLLING
    pthread_create (NULL, NULL, wait_foo, NULL);
#endif
    /* Now we allocate some memory for the dispatch context
     * structure, which will later be used when we receive
     * messages. */
    ctp = dispatch_context_alloc (dpp);

   /* background the process */
//   procmgr_daemon(0, PROCMGR_DAEMON_NOCLOSE|PROCMGR_DAEMON_NODEVNULL);

    /* Done! We can now go into our "receive loop" and wait
     * for messages. The dispatch_block() function is calling
     * MsgReceive() under the covers, and receives for us.
     * The dispatch_handler() function analyzes the message
     * for us and calls the appropriate callback function. */
    while (!done) {
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
    return 0;
}

/*
 *  io_open
 *
 * We are called here when the client does an open().
 * Set Rx and Tx interrupt, and mark in device bitmask as opened
 */

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
		void *extra)
{
	if (optv)
	{
		printf("%s:  in io_open\n", progname);
	}
	if (CheckBit(dev_l.dev_open, ctp->id))
	{
		return EBUSY;
	}
	else
	{
		fifo_init(&channel[ctp->id].rx_fifo);
		fifo_init(&channel[ctp->id].tx_fifo);
		from_config(ctp->id);
		dev_l.dev_open |= (1 << ctp->id);
		p_uart[ctp->id]->ier_dlh = p_uart[ctp->id]->ier_dlh | IER_RxD_Set;
		return (iofunc_open_default(ctp, msg, handle, extra));
	}
}

/*
 *  io_close
 *
 * We are called here when the client does an close().
 * Clear Rx and Tx interrupt, and mark in bitmask dev/rts/RS* as closed
 */

int io_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
	if (optv)
	{
		printf("%s:  in io_open\n", progname);
	}
	dev_l.dev_open &= ~(1 << ctp->id);
	p_uart[ctp->id]->ier_dlh = 0;

	return (close(ctp->id));
}


/*
 *  options
 *
 *  This routine handles the command-line options.
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

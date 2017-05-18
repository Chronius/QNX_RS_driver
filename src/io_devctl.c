/*
 * io_devctl.c
 *
 *  Created on: 18 мая 2017 г.
 *      Author: abuzarov_bv
 */
#include <proto.h>

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

//      This file is part of iCP12 by BayCom GmbH.
//
//      iCP12 is free software: you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation, either version 3 of the License, or
//      (at your option) any later version.
//
//      iCP12 is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with iCP12.  If not, see <http://www.gnu.org/licenses/>.
//
//      The developer can be reached at software@baycom.tv


#include <stdio.h>		/* Standard input/output definitions */
#include <string.h>		/* String function definitions */
#include <unistd.h>		/* UNIX standard function definitions */
#include <fcntl.h>		/* File control definitions */
#include <errno.h>		/* Error number definitions */
#include <termios.h>		/* POSIX terminal control definitions */
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

void showversion(void)
{
	fprintf (stderr, "iCP12 -d <device path> -p <port> -c <channel> -a|-i|-o <val>|-m i|-m o|-m a -v\n\n" "     Examples: \n" "     Read analog value from port AN0 : iCP12 -p A -c 0 -a\n" "     Read digital value from port RB4: iCP12 -p B -c 4 -i\n" "     Write digital value to port RC6 : iCP12 -p C -c 6 -o 1\n" "     Configure RA1 for analog input  : iCP12 -p A -c 1 -m a\n" "                   ... digital input : iCP12 -p A -c 1 -m i\n" "                   ... digital output: iCP12 -p A -c 1 -m o\n");
}

static int fd=-1;			/* File descriptor for the port */

void icp12 (char *device, char *cmd, char *receive, int reclen)
{
	int n;
	int bytes = -1;

	struct termios options;
	if(fd==-1) {
		fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);

		if (fd == -1) {
			fprintf (stderr, "Cannot open device: %s\n", device);
			return;
		}

		tcgetattr (fd, &options);

		/* SEt Baud Rate */
		cfsetispeed (&options, B115200);
		cfsetospeed (&options, B115200);

		// Enable Read
		options.c_cflag |= (CLOCAL | CREAD);

		// Set the Charactor size
		options.c_cflag &= ~CSIZE;	/* Mask the character size bits */
		options.c_cflag |= CS8;	/* Select 8 data bits */

		// Set parity - No Parity (8N1)
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;

		// Enable Raw Input
		options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

		// Disable Software Flow control
		options.c_iflag &= ~(IXON | IXOFF | IXANY);

		// Chose raw (not processed) output
		options.c_oflag &= ~OPOST;

		if (tcsetattr (fd, TCSANOW, &options) == -1) {
			fprintf (stderr, "Error with tcsetattr = %s\n", strerror (errno));
		}
	}
	// Write to the port
	int len = strlen (cmd);
	n = write (fd, cmd, len);
	if (n != len) {
		fprintf (stderr, "write() of command %s failed!\n", cmd);
	}

	memset (receive, 0, reclen);
	usleep (5000);

	time_t t = time (NULL);
	while ((time (NULL) - t) < 1) {
		bytes = read (fd, receive, reclen);
		if (bytes > 0) {
			break;
		}
		usleep (100000);
	}
	if (bytes <= 0) {
		fprintf (stderr, "Error reading from device, command: %s\n", cmd);
	}
}


int main (int argc, char **argv)
{
	char c;
	int ch = 0;
	char *device = "/dev/ttyACM0";
	char mode = 0;
	char cmd[32];
	char recv[32];
	int val = 0;
	char port = 'A';

	while (1) {
		c = getopt (argc, argv, "hc:d:c:aio:m:p:v");
		if (c == -1)
			break;

		switch (c) {
		case 'd':
			device = optarg;
			break;
		case 'c':
			ch = atoi (optarg);
			break;
		case 'p':
			port = toupper (optarg[0]);
			break;
		case 'a':
			sprintf (cmd, "(A%c%dc)", port, ch);
			icp12 (device, cmd, recv, sizeof (recv));
			if (strlen (recv) != 1 || recv[0] != '#') {
				fprintf (stderr, "Command failed: %s\n", cmd);
			}
			sprintf (cmd, "(A%c%dr)", port, ch);
			icp12 (device, cmd, recv, sizeof (recv));
			if (strlen (recv) == 13) {
				recv[12] = 0;
				printf ("AN%d=%d\n", ch, atoi (recv + 6));
			} else {
				fprintf (stderr, "Command failed: %s\n", cmd);
			}
			break;
		case 'i':
			sprintf (cmd, "(DP%c)", port);
			icp12 (device, cmd, recv, sizeof (recv));
			if (strlen (recv) == 13) {
				printf ("P%c%d=%c\n", port, ch, recv[4 + ch]);
			} else {
				fprintf (stderr, "Command failed: %s\n", cmd);
			}
			break;
		case 'o':
			val = atoi (optarg);
			sprintf (cmd, "(R%c%d%d)", port, ch, val);
			icp12 (device, cmd, recv, sizeof (recv));
			if (strlen (recv) != 1 || recv[0] != '#') {
				fprintf (stderr, "Command failed: %s\n", cmd);
			} else {
				printf ("OK\n");
			}
			break;
		case 'm':
			mode = tolower (optarg[0]);
			switch (mode) {
			case 'i':
				sprintf (cmd, "(R%c%di)", port, ch);
				icp12 (device, cmd, recv, sizeof (recv));
				break;
			case 'o':
				sprintf (cmd, "(R%c%do)", port, ch);
				icp12 (device, cmd, recv, sizeof (recv));
				break;
			case 'a':
				sprintf (cmd, "(R%c%da)", port, ch);
				icp12 (device, cmd, recv, sizeof (recv));
				break;
			default:
				fprintf (stderr, "Invalid mode %c for channel %d, try i,o or a\n", mode, ch);
			}
			if (strlen (recv) != 1 || recv[0] != '#') {
				fprintf (stderr, "Command failed: %s\n", cmd);
			} else {
				printf ("OK\n");
			}
			break;
		case 'v':
			printf ("i2P12 utility version 1.0\n");
			sprintf (cmd, "(ZMD)");
			icp12 (device, cmd, recv, sizeof (recv));
			int len = strlen (recv);
			if (len > 5) {
				recv[len - 1] = 0;
				printf ("Device version %s\n", recv + 5);
			}
			break;
		default:
			showversion();
		}
	}
	if(argc == 1) {
		showversion();
	} 
	
	if(fd!=-1) {
		close (fd);
	}
	return 0;
}

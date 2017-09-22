#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>


static int keep_running = 1;

void int_handler(int signo);
void int_handler(int signo)
{
    (void)signo;

    keep_running = 0;
}

int main(int argc, char* argv[])
{
    int fd = 0;
    struct termios options;
    struct sigaction act;

    if (argc != 2) {
        printf("Usage: %s <path to serialport>", argv[0]);
        exit(1);
    }

    printf("Opening %s\n", argv[1]);

    /* If the CLOCAL flag for a line is off, the hardware carrier detect (DCD) signal
       is significant, and an open(2) of the corresponding terminal will block until DCD
       is asserted, unless the O_NONBLOCK flag is given. */
    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) {
        perror("Open port");
    }

    /* Make port exclusive. Prevent additional opens. */
    ioctl(fd, TIOCEXCL);

    /* Now that the device is open, clear the O_NONBLOCK flag so subsequent I/O will block. */
    fcntl(fd, F_SETFL, 0);

    /* Config */
    tcgetattr(fd, &options);
    cfmakeraw(&options);
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 10;
    cfsetspeed(&options, B57600);
#ifdef __linux__
    options.c_cflag |= (CLOCAL | CREAD | CRTSCTS);
#else
    options.c_cflag |= (CLOCAL | CREAD | CS8 | CCTS_OFLOW | CRTS_IFLOW);
#endif

#ifndef __linux__
    /* Assert DTR */
    ioctl(fd, TIOCSDTR);
#endif

    /* Commit */
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &options);

    act.sa_handler = int_handler;
    sigaction(SIGINT, &act, NULL);
    while (keep_running) {
        fflush(stdout);
        write(fd, "0123456789", 10);
    }

    printf("\nctrl-c\n");

#ifndef __linux__
    /* Clear DTR */
    ioctl(fd, TIOCCDTR);
#endif

    close(fd);

    return 0;
}

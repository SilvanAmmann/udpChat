#include <netdb.h>      /* network database library */
#include <sys/socket.h> /* sockets */
#include <arpa/inet.h>  /* address conversions */
#include <unistd.h>
#include <string.h>   /* memset, strlen */
#include <stdio.h>    /* printf */
#include <gpiod.h>
#include <stdlib.h>  /* for atoi */
#include <Python.h>

#define BUFSIZE (10*1024) /* size of buffer, max 64 KByte */
#define DEFAULT_LED_PIN  21  /* Default pin if no argument is provided */
static unsigned char buf[BUFSIZE]; /* receive buffer */
float temperature;
void temp() {
    printf("Temp start\n");

    // Set PYTHONPATH to the working directory
    setenv("PYTHONPATH", ".", 1);

    PyObject *pName, *pModule, *pFunc, *pValue;

    Py_Initialize();

    // Import the Python module
    PyRun_SimpleString("import sys; sys.path.append('.')");

    // Import the Python module named 'sht31'
    pName = PyUnicode_DecodeFSDefault("sht31");
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        // Get the function named 'temp'
        pFunc = PyObject_GetAttrString(pModule, "temp");
        if (pFunc && PyCallable_Check(pFunc)) {
            // Call the function and get the result
            pValue = PyObject_CallObject(pFunc, NULL);
            if (pValue != NULL) {
                temperature = (float)PyFloat_AsDouble(pValue);
                printf("Temperature: %.2f C\n", temperature); 
                Py_DECREF(pValue);
            } else {
                PyErr_Print();
                fprintf(stderr, "Call failed\n");
            }
        } else {
            PyErr_Print();
            fprintf(stderr, "Cannot find function 'temp'\n");
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    } else {
        PyErr_Print();
        fprintf(stderr, "Failed to load module 'sht31'\n");
    }

    Py_Finalize();
}

void blink(){
  const char *chipname = "gpiochip0";
  struct gpiod_chip *chip;
  struct gpiod_line *led;
  int led_pin = DEFAULT_LED_PIN;

  printf("Blinky LED example with gpiod on pin %d\n", led_pin);

  // Open the GPIO chip
  chip = gpiod_chip_open_by_name(chipname);
  if (!chip) {
      perror("Open GPIO chip failed");
  }

  // Get the LED line
  led = gpiod_chip_get_line(chip, led_pin);
  if (!led) {
      perror("Get LED line failed");
      gpiod_chip_close(chip);
  }

  // Request the LED line as output
  if (gpiod_line_request_output(led, "led", 0) < 0) {
      perror("Request LED line as output failed");
      gpiod_line_release(led);
      gpiod_chip_close(chip);
  }

  // Blink the LED
  for (int i = 0; i < 50; i++) {
      gpiod_line_set_value(led, (i & 1) != 0);  // Toggle LED
      usleep(100 * 1000);                      // Wait 100 ms
  }

  gpiod_line_set_value(led, 0);  // Turn off LED

  // Release handles
  gpiod_line_release(led);
  gpiod_chip_close(chip);
}

int main(void) {
  struct sockaddr_in myaddr;  /* our address */
  struct sockaddr_in remaddr; /* remote address */
  socklen_t addrlen = sizeof(remaddr);    /* length of addresses */
  int recvlen;      /* # bytes received */
  int fd;       /* socket file descriptor */
  int msgcnt = 0;     /* count # of messages we received */
  const int port = 1234; /* port used for socket */

  /* create a UDP socket */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("cannot create socket\n");
    return 0;
  }

  /* bind the socket to any valid IP address and a specific port */
  memset((char *)&myaddr, 0, sizeof(myaddr)); /* initialize all fields */
  myaddr.sin_family = AF_INET; /* Internet protocol */
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY); /* accept all incoming */
  myaddr.sin_port = htons(port); /* port number for socket */

  if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
    perror("bind failed");
    return 0;
  }

  /* now loop, receiving data and printing what we received */
  for (;;) {
    printf("waiting on port %d\n", port);
    recvlen = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&remaddr, &addrlen);
    if (recvlen > 0) {
      buf[recvlen] = '\0'; /* terminate buffer */
      printf("received message: \"%s\" (%d bytes)\n", buf, recvlen);
      blink();
      temp();
    } else {
      printf("uh oh - something went wrong!\n");
    }
    sprintf((char*)buf, "temp = %.2f", temperature); /* prepare answer */
    printf("sending response \"%s\"\n", buf);
    if (sendto(fd, buf, strlen((char*)buf), 0, (struct sockaddr *)&remaddr, addrlen) < 0) {
      perror("sendto");
    }
  }
  close(fd);
  return 0;
}

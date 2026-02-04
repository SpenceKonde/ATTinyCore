/*  checkthis.c  */
#include <stdio.h>
#include <string.h>
#if defined WIN
  #include <lusb0_usb.h>    // this is libusb, see http://libusb.sourceforge.net/
#else
  #include <usb.h>        // this is libusb, see http://libusb.sourceforge.net/
#endif
#include <curses.h>
#include <stdlib.h>


int main (int argc, char **argv)
{


  bool sendLine = true;
  int arg_pointer = 1;


  while (arg_pointer < argc) {

    if(strcmp(argv[arg_pointer], "--no-new-line") == 0) {
      sendLine = false;
    }

    arg_pointer++;
  }
 


  struct usb_bus *bus = NULL;
  struct usb_device *digiSpark = NULL;
  struct usb_device *device = NULL;

  // Initialize the USB library
  usb_init();

  // Enumerate the USB device tree
  usb_find_busses();
  usb_find_devices();

  // Iterate through attached busses and devices
  bus = usb_get_busses();
  while(bus != NULL)
  {
     device = bus->devices;
     while(device != NULL)
     {
        // Check to see if each USB device matches the DigiSpark Vendor and Product IDs
        if((device->descriptor.idVendor == 0x16c0) && (device->descriptor.idProduct == 0x05df))
        {
           digiSpark = device;
        }

        device = device->next;
     }

     bus = bus->next;
  }
if(digiSpark == NULL)
  {
  printf("No Digispark Found");
    return 1;
}

int result = 0;
     
     int numInterfaces = 0;
     struct usb_dev_handle *devHandle = NULL;
     struct usb_interface_descriptor *interface = NULL;
    
        
        devHandle = usb_open(digiSpark);
         
        if(devHandle != NULL)
        {
           /*result = usb_set_configuration(devHandle, digiSpark->config->bConfigurationValue);
           if(result < 0) {printf("Error %i setting configuration to %i\n", result, digiSpark->config->bConfigurationValue); return 1;}*/
        
           numInterfaces = digiSpark->config->bNumInterfaces;
           interface = &(digiSpark->config->interface[0].altsetting[0]);
           //if(debug) printf("Found %i interfaces, using interface %i\n", numInterfaces, interface->bInterfaceNumber);
   
           /*result = usb_claim_interface(devHandle, interface->bInterfaceNumber);
           if(result < 0) { printf("Error %i claiming Interface %i\n", result, interface->bInterfaceNumber); return 1;}*/
  

        }

    WINDOW *w, * childwin, * inputwin, * topwin;
    char c;
    char thechar = ' ';
    char input[8];
    w = initscr();
    //noecho();
    nodelay(stdscr, TRUE);
    noecho();
    curs_set(0);

    topwin = subwin(w, 1, COLS, 0, 0);

    childwin = subwin(w, LINES-3, COLS, 2, 0);
    inputwin = subwin(w, 1, COLS, LINES-1, 0);
    scrollok(childwin, true);



    refresh();


  // move(LINES-1,0);
  // printw(">"); 
    waddstr(topwin, "DIGISPARK DIGIUSB TERMINAL - Sends data to Digispark on return. Ctrl-C to Exit");
    wrefresh(topwin);
    waddstr(inputwin, ">");
    wrefresh(inputwin);
    int i = 0;
    int a = 0;
    int stringLength;

   while (c != 3) { 
		c = getch();      // retrieve xtended scancode
		if(c != ERR){

			if (c == 10){

				if(sendLine)
					input[a] = '\n';

				a++;
				stringLength = a;
				i=0;

				int y, x;
				getyx(childwin, y, x);


				if(x!=0)
					wprintw(childwin, "\n");

				waddstr(childwin, ">");
				while(i < stringLength){
					wprintw(childwin, "%c", input[i]);
	            	result = usb_control_msg(devHandle, (0x01 << 5), 0x09, 0, input[i], 0, 0, 1000);
	            	//printf("Writing character \"%c\" to DigiSpark.\n", input[i]);
	            	if(result < 0) {printf("Error %i writing to USB device\n", result); return 1;}
	            	i++;
	          	}

	          	if(!sendLine)
	          		wprintw(childwin, "\n");

				//
				
				wrefresh(childwin);
				wclear(inputwin);
				waddstr(inputwin, ">");
				wrefresh(inputwin);

				a=0;
	      input[0] = '\0';


				
				
			}
			else{

				input[a] = c;
				a++;
				wprintw(inputwin, "%c", c);
				wrefresh(inputwin);

			}
		}
		else{
			thechar = ' ';

			while(thechar != 4)
		      {

		        thechar = 4;
		        result = usb_control_msg(devHandle, (0x01 << 5) | 0x80, 0x01, 0, 0, &thechar, 1, 1000);
		        if(result > 0)
		        {
			        if(thechar != 4){
			        	if(thechar=='\n'){
                  int y, x;
                  getyx(childwin, y, x);

                  if(y==LINES-4)
                    wscrl(childwin,1);
                  else
    							 result = wmove(childwin, y + 1, x);
			        	}
			         else{
			           waddch(childwin, thechar);

               }
                
			          
		
			        }
		        }
            usleep(100000);
		      }
		      wrefresh(childwin);
          refresh();

		}
    #if defined WIN
    Sleep(100);
    #else
    usleep(100000);
    #endif


	}

	result = usb_release_interface(devHandle, interface->bInterfaceNumber);
           if(result < 0) {printf("Error %i releasing Interface 0\n", result); return 1;}

           usb_close(devHandle);

    endwin();

    //printf("received %c (%d)\n", c, (int) c);
}


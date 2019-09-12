/* 
 * orbit.c   Solar System viewer
 * k theis 9/2019
 * 
 * This program displays some of the bodies in the solar system
 * onto the frame buffer display. This is NOT written for X-windows.
 * This was written for linux on the raspberry pi 4 as a test of the 
 * computers speed. It seems to do very well.
 * 
 * compile with cc -o orbit orbit.c -lm
 * 
 * NOTE: This is a toy. Please do not launch spacecraft based on it's predictions.
 * 
 * With a speed of 6.0, simulation time is appx 3 1/2 days per second of real time.
 * With a zoom factor of 200 you will see to almost the orbit of Jupiter (about 4 A.U.).
 * 
 * On the display, the central object is the Sun. The planets are slightly smaller white points.
 * Asteroids are variable bright white to gray points. Comets are blueish points. 
 * 
 * 

==================================================================================================================

  from https://ssd.jpl.nasa.gov/?sb_elem

Named Asteroids (Using the first 60,000 or so)

Column headings:
Num	 			Number assigned to the asteroid or comet. Numbers are only assigned to secure short-period (<200 year) comets.
Name	 		Name given to the numbered asteroid or comet.
Designation	 	Unnumbered asteroid's preliminary designation.
Epoch			(MJD)	Epoch of the elements represented as the Modified Julian Date (MJD), which is defined as the Julian date - 2400000.5.
a				(AU)	Semimajor axis of the orbit (asteroids only).
q				(AU)	Perihelion distance (comets only).
e	 			Eccentricity of the orbit.
i				(deg.)	Inclination of the orbit with respect to the J2000 ecliptic plane.
w				(deg.)	Argument of perihelion (J2000-Ecliptic).
Node			     (deg.)	Longitude of the ascending node (J2000-Ecliptic).
M				(deg.)	Mean anomaly at epoch (asteroids only).
Tp	 			Time of perihelion passage (comets only), formatted as a calendar date (YYYYMMDD.DDD) where "YYYY" is the year, "MM" is the numeric month, and "DD.DDD" is the day and day fraction.
H				(mag.)	Absolute magnitude (asteroids only).
G	 			Magnitude slope parameter (asteroids only).
Ref	 			Orbit solution reference.

Example:

Num   Name              Epoch      a          e        i         w        Node        M         H    G   Ref
------ ----------------- ----- ---------- ---------- --------- --------- --------- ----------- ----- ---- ----------
     1 Ceres             55400  2.7653485 0.07913825  10.58682  72.58981  80.39320 113.4104434  3.34 0.12 JPL 30
     2 Pallas            55400  2.7721532 0.23099956  34.84090 310.15094 173.12950  96.1482660  4.13 0.11 JPL 25
     3 Juno              55400  2.6700912 0.25498122  12.98211 248.10807 169.91138  32.0960832  5.33 0.32 JPL 86


=================================================================================================================


From https://ssd.jpl.nasa.gov/?sb_elem

Named Comets (using the first 360)


Sample Comet File Header
Num  Name                                     Epoch      q           e        i         w        Node          Tp       Ref
------------------------------------------- ------- ----------- ---------- --------- --------- --------- -------------- ------------
  1P/Halley                                   49400  0.58597811 0.96714291 162.26269 111.33249  58.42008 19860205.89532 JPL J863/77
  2P/Encke                                    55460  0.33586872 0.84833156  11.78308 186.54970 334.56582 20100806.50196 JPL K105/1
  3D/Biela                                    -9480  0.87907300 0.75129900  13.21640 221.65880 250.66900 18321126.61520 IAUCAT03
 
See above for column heading info.
 
================================================================================================================= 
 
From https://www-istp.gsfc.nasa.gov/stargaze/Lkepl1st.htm

Equations:
  x = r cos(theta)
  y = r sin(theta)
  r^2 = x^2 + y^2
  tan(theta) = y/x
Using f as theta:
  equasion for an elipse is:
  r = a(1-e^2) / (1+e cos f)
  w/e is ellipse between 0 and 1 (eccentricity of an elipse)
  and a is the radius
  
    
==================================================================================================================    
    
    
From http://www.met.rdg.ac.uk/~ross/Astronomy/Planets.html

Planetary orbital elements for the major planets
  
 
===================================================================================================================
   

*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <signal.h>


#define ASTCOUNT 60000   // Number of imported asteroids (starting from 1)
#define COMETCOUNT 380   // Number of imported comets
#define SPEED 5.0       // simulation speed. Smaller #'s (down to 1.0) are faster, larger (up to 10.0) are slower
#define OBJCOUNT ASTCOUNT+COMETCOUNT+50      // Number of imported objects (50 is max number of hand-imported objects)

// Global Variables
int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;           // frame buffer pointer
long int location = 0;   // used for plotting
int EXITFLAG = 0;        // 0 until ctrl-c/SIGTERM caught
int exit_signal = 0;     // 0 until ctrl-c/SIGTERM caught

// SIGTERM caught - exit gracefully
void termination_handler (int signum) {
     EXITFLAG = 1;
     exit_signal = 1;
}

// ctrl-c sig handler
void ctrlc_handler (int signum) {
     EXITFLAG = 1;
     exit_signal = 1;
}


// Clear the frame buffer (set everything to black)
int clear(void) {
     int y, x;
     
     
     for (y=0; y<vinfo.yres; y++) {
	    for (x=0; x<vinfo.xres; x++) {
		    location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
		    	       (y+vinfo.yoffset) * finfo.line_length;
		    if (vinfo.bits_per_pixel == 32) {	// normally all 0 for black
			    *(fbp + location + 0) = 0;	// Blue
			    *(fbp + location + 1) = 0;	// Green
			    *(fbp + location + 2) = 0;	// Red
			    *(fbp + location + 3) = 0;	// No transparency
		    }
		    else {
			    printf("Error - framebuffer size != 32\n");
			    exit(1);
		    }
	   }
    }
    return(0);
}

// plot/unplot a point/body w/variable magnitude
int plotxy(int xloc, int yloc, float mag, int body) {
     int x, y;
	// bounds check first
	if (xloc-body < 0) return 1;
	if (yloc-body < 0) return 1;
	if (xloc+body > vinfo.xres-1) return 1;
	if (yloc+body > vinfo.yres-1) return 1;
     
     if (mag == 50) {         // clear the point
     for (x=0; x<=body; x++) 
          for (y=0; y<=body; y++) {
               
               location = (x+xloc+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
               (y+yloc+vinfo.yoffset) * finfo.line_length;

               /* clear the point (all 0 is black) */
               *(fbp + location + 0) = 0;	// Blue
               *(fbp + location + 1) = 0;	// Green
               *(fbp + location + 2) = 0;	// Red
               *(fbp + location + 3) = 0;	// No transparency

          }
     } else if (mag == 60) {  // plot comets (colored points)
          for (x=0; x<=body; x++)
               for (y=0; y<=body; y++) {
                    location = (x+xloc+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                    (y+yloc+vinfo.yoffset) * finfo.line_length;

               
               *(fbp + location + 0) = 255;	// Blue
               *(fbp + location + 1) = 0;	// Green
               *(fbp + location + 2) = 128;	// Red
               *(fbp + location + 3) = 0;	// No transparency
               }
          
     } else {
     // plot it
     for (x=0; x<=body; x++) 
          for (y=0; y<=body; y++) {
               
               location = (x+xloc+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
               (y+yloc+vinfo.yoffset) * finfo.line_length;
               
               if (mag >= 0.0) {
                    /* color white */
                    // normally 255,255,255 for all white
                    *(fbp + location + 0) = (255-(mag*mag));	// Blue
                    *(fbp + location + 1) = (255-(mag*mag));	// Green
                    *(fbp + location + 2) = (255-(mag*mag));	// Red
                    *(fbp + location + 3) = 0;	// No transparency
               }
               
          }
     }
      
      return 0;
}


// usage: orbit [zoom] where zoom is between 30 & 400 for typical usage
int main(int argc, char **argv) {  
     
     FILE *infile;
     
     int FLAG = 0;
     
     // used to process objects
     float* a = malloc(sizeof(float) * OBJCOUNT);      if (a == NULL) FLAG=1;;
     float* e = malloc(sizeof(float) * OBJCOUNT);      if (e == NULL) FLAG=1;
     float* mag = malloc(sizeof(float) * OBJCOUNT);    if (mag == NULL) FLAG=1;
     float* node = malloc(sizeof(float) * OBJCOUNT);   if (node == NULL) FLAG=1;
     
     // Bodysize: 0=point, 1=planet, 2=sun
     int* bodysize = malloc(sizeof(int) * OBJCOUNT);   if (bodysize == NULL) FLAG=1;
     
     if (FLAG == 1) {
          fprintf(stderr,"Memory allocation error\n");
          exit(1);
     }

     // general purpose vars
     float x, y;
     float r, theta;
     
     // used to read in from file
     int Num, RefNum, Epoch, i;
     char Name[40],RefName[10];
     float A,E,I,W,Node,M,H,G;
     char line[135];	// read line from file
     
     // misc variables
     float period = 0.0; // determine orbital speed
     int cnt = 0;        // object count
     int maxcount = 0;   // total objects counted
     int ZOOM;           // max viewing area
     
     // test command line
     if (argc == 2) {
          ZOOM = atoi(argv[1]);
          if (ZOOM == 0) {    // show usage and exit
               fprintf(stderr,"Usage: %s [zoom value]\n",argv[0]);
               fprintf(stderr,"where zoom value is between 30 and 300\n");
               exit(0);
          }
     } else {
          ZOOM = 220;    // 220 is appx 4 A.U. in diameter, just inside Jupiters orbit
     }

     // sig handler (catch ctrl-c for graceful closing)
     if (signal (SIGINT, ctrlc_handler) == SIG_IGN) {
          signal(SIGINT, SIG_IGN);
     }


     cnt = 0;
     
     // Predefine major bodies
     a[cnt] = 0.0; e[cnt] = 0.0; node[cnt] = 0.0; mag[cnt] = 0; bodysize[cnt] = 2; cnt++;  // Sun
     a[cnt] = 0.387; e[cnt] = 0.2056; node[cnt] = 252.0; mag[cnt] = 0; bodysize[cnt] = 1; cnt++; // Mercury
     a[cnt] = 0.723; e[cnt] = 0.006; node[cnt] = 45.0; mag[cnt] = 0; bodysize[cnt] = 1; cnt++; // Venus
     a[cnt] = 1.0; e[cnt] = 0.017; node[cnt] = 90.0; mag[cnt] = 0; bodysize[cnt] = 1; cnt++; // Earth
     a[cnt] = 1.52; e[cnt] = 0.0934; node[cnt] = 355.4; mag[cnt] = 0; bodysize[cnt] = 1; cnt++; // Mars
     a[cnt] = 5.2; e[cnt] = 0.0484; node[cnt] = 34.4; mag[cnt] = 0; bodysize[cnt] = 1; cnt++; // Jupiter
     a[cnt] = 9.54; e[cnt] = 0.054; node[cnt] = 49.9; mag[cnt] = 0; bodysize[cnt] = 1; cnt++; // Saturn
     a[cnt] = 19.19; e[cnt] = 0.047; node[cnt] = 313.2; mag[cnt] = 0; bodysize[cnt] = 1; cnt++; // Uranus
     a[cnt] = 30.06896; e[cnt] = 0.0086; node[cnt] = 304.88; mag[cnt] = 0; bodysize[cnt] = 1; cnt++; // Neptune
     a[cnt] = 39.4817; e[cnt] = 0.2488; node[cnt] = 238.928; mag[cnt] = 0; bodysize[cnt] = 1; cnt++; // Pluto
 
     // If you want to play with orbital patterns you can add objects below (max 40)
     //a[cnt] = 1.5; e[cnt] = .65; node[cnt] = 0.0; mag[cnt] = 0; bodysize[cnt] = 1; cnt++;     // test object 1
 
     // ***** read in asteroid file *****
     infile = fopen("ELEMENTS.NUMBR","r");
     if (infile == NULL) {
          fprintf(stderr,"\nError - failed to open asteroid file ELEMENTS.NUMBR\n");
          exit(1);
     }

     fgets(line,sizeof(line),infile);   // read/discard 1st line from file
     fgets(line,sizeof(line),infile);   // and second line
     // read & enter asteroids up to ASTCOUNT
     for (i=0; i<ASTCOUNT; i++) {
          fgets(line,sizeof(line),infile);
          if (line[strlen(line)-1]=='\n') line[strlen(line)-1]='\0';
          sscanf(line," %d %s %d %f %f %f %f %f %f %f %f %s %d", \
          &Num,Name,&Epoch,&A,&E,&I,&W,&Node,&M,&H,&G,RefName,&RefNum);
          if (H > 15.5)  continue;     // skip dim objects (15.5 is dimmest w/current mag calc in plotxy)
          a[cnt] = A;
          e[cnt] = E;
          node[cnt] = Node;
          mag[cnt] = H;
          bodysize[cnt] = 0;  // for asteroids etc
          cnt++;
          
     }


     // ***** read in comet file *****
     infile = fopen("ELEMENTS.COMET","r");
     if (infile == NULL) {
          fprintf(stderr,"\nError - failed to open comets file ELEMENTS.COMET\n");
          exit(1);
     }

     fgets(line,sizeof(line),infile);   // read/discard 1st line from file
     for (i=0;  i<COMETCOUNT; i++) {          // read & enter comets up to COMETCOUNT
          fgets(line,sizeof(line),infile);
          if (line[strlen(line)-1]=='\n') line[strlen(line)-1]='\0';
          sscanf(line," %s %d %f %f %f %f %f %f %f %f %s %d", \
          Name,&Epoch,&A,&E,&I,&W,&Node,&M,RefName);
          if (A < 1.5) continue;   // ignore comets inside Mars orbit (1.5 A.U.)
          a[cnt] = A;
          e[cnt] = E;
          node[cnt] = Node;
          mag[cnt] = 60;      // default for comets
          bodysize[cnt] = 0;  // for comets etc
          cnt++;
     }


     maxcount = cnt;     
               
     // Open the frame buffer for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    // The framebuffer device was opened successfully
    
    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }
    
    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }
    
    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    if (vinfo.bits_per_pixel != 32) {
         fprintf(stderr,"This program uses %d bits per pixel.\n",vinfo.bits_per_pixel);
         fprintf(stderr,"It requires 32 pits per pixel. Stopping.\n");
         exit(0);
     }
    
    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    // The framebuffer device was mapped to memory successfully

    // clear the frame buffer (set everything to black)
    clear();
    
    
 
     //*** Main Loop ***//
     
     // plot objects
     while (1)  {
          for (cnt=0; cnt<=maxcount; cnt++) {
          
          // clear the old point
          r = ((a[cnt]*ZOOM)*(1-e[cnt]*e[cnt])) / (1+e[cnt]*cos(node[cnt] * 0.01745));
          x = r * cos(node[cnt] * 0.01745);      // convert to radians
          y = r * sin(node[cnt] * 0.01745);
          //x = x+(vinfo.xres/2);		// center it
          //y = y+(vinfo.yres/2);		// center it
          plotxy(x+(vinfo.xres/2),y+(vinfo.yres/2),50.0,bodysize[cnt]);    // clear old point, use mag 50.0 as flag
          
          // update object position
          //Normal: period = sqrt(pow(a[cnt],3));
          // but when e is > 0 this doesn't apply. r is the true a so we apply that:
          r = ((a[cnt])*(1-e[cnt]*e[cnt])) / (1+e[cnt]*cos(node[cnt] * 0.01745));
          period = sqrt(pow(r,3));
          node[cnt] += 1.0/(period * SPEED); // if SPEED=1.0 there is no delay
          if (node[cnt] > 359.99) node[cnt] = 0.0;
          
          // plot the new point
          r = ((a[cnt]*ZOOM)*(1-e[cnt]*e[cnt])) / (1+e[cnt]*cos(node[cnt] * 0.01745));
          x = r * cos(node[cnt] * 0.01745);      // convert to radians
          y = r * sin(node[cnt] * 0.01745);
          //x = x+(vinfo.xres/2);		// center it
          //y = y+(vinfo.yres/2);		// center it
          plotxy(x+(vinfo.xres/2),y+(vinfo.yres/2),mag[cnt],bodysize[cnt]);     // show new point

          }
          
          // test ctrl-c and SIGINT
          if (EXITFLAG == 1) break;
          //usleep(3000);  // time between updates (used during testing)
         
     }
     
      /* Done with program - clean up */

    fprintf(stderr,"Shutting down %s\n",argv[0]);
    clear();	               // clear the display before closing
    free(a);                  // free up memory
    free(e);
    free(mag);
    free(node);
    free(bodysize);
    munmap(fbp, screensize);  // freeup the screen memory
    close(fbfd);              // close the frame buffer pointer
    fprintf(stderr,"Program exited\n");
    return 0;
}
          

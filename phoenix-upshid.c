/*
	MIT License
	
	Copyright (c) 2022 DrMaxNix
	
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	
	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
	
	Inspired by:
		- https://github.com/torvalds/linux/blob/master/samples/hidraw/hid-example.c
		- https://github.com/gregkh/usbutils/blob/master/usbreset.c
		- https://askubuntu.com/questions/645/how-do-you-reset-a-usb-device-from-the-command-line
*/

#include <linux/usbdevice_fs.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>


// MACROS //
// convert boolean to string (true/false)
#define bool_string(value) (value ? "true" : "false")


// FUNCTIONS WE WILL DEFINE LATER //
int find_device_hid(uint16_t vendor_id, uint16_t device_id);
int find_device_bus(uint16_t vendor_id, uint16_t device_id);

void output_data();
void usbreset();

void print_help();
void print_version();


// PUBLIC FLAGS //
bool auto_usbreset = false;

uint16_t vendor_id;
uint16_t product_id;


/*
	Main
*/
int main(int argc, char **argv){
	// PARSE FLAGGS //
	int one_flag;
	while((one_flag = getopt(argc, argv, "vha")) != -1){
		switch(one_flag){
			// automatically try usb reset
			case 'a':
				auto_usbreset = true;
				break;
				
			// version
			case 'v':
				print_version();
				break;
				
			// help
			case 'h':
				print_help(argv[0]);
				break;
				
			// option error
			case '?':
				exit(1);
				break;
			
			// should never reach this..
			default:
				abort();
				break;
		}
	}
	
	
	// OTHER ARGUMENTS //
	// check count
	if(argc - optind != 1){
		fprintf(stderr, "[ERROR] parameter count must be 1; use -h flag for help\n");
		exit(1);
	}
	
	// parse
	for(size_t q = optind; q < argc; q++){
		if(q == optind){
			// make sure string has a collon in it
			char *arg_id = argv[q];
			if(!strstr(arg_id, ":")){
				fprintf(stderr, "[ERROR] parameter 1 must contain a collon; use -h flag for help\n");
				exit(1);
			}
			
			// devide by collon and do hex-string decode
			vendor_id = strtol(strsep(&arg_id, ":"), NULL, 16);
			product_id = strtol(strsep(&arg_id, ":"), NULL, 16);
		}
	}
	
	
	// READ AND OUTPUT DATA //
	output_data();
	
	
	// EXIT //
	return(0);
}

/*
	HELPER: Find hid device file of form '/dev/hidrawx' for device with SEARCH-VENDOR-ID and SEARCH-PRODUCT-ID and return its handle
*/
int find_device_hid(uint16_t search_vendor_id, uint16_t search_product_id){
	// OPEN /DEV AS DIR //
	DIR *dir = opendir("/dev");
	struct dirent *dirent;
	
	
	// CHECK IF READABLE //
	if(dir == NULL){
		// not readable
		fprintf(stderr, "[ERROR] /dev not readable\n");
		exit(0);
	}
	
	
	// check every file in dir
	while((dirent = readdir(dir)) != NULL){
		// CHECK FILETYPE //
		if(dirent->d_type != DT_CHR){
			// not a char device, try next one
			continue;
		}
		
		
		// CHECK FILENAME //
		// check filename length
		if(strlen(dirent->d_name) <= 6){
			// filename too short, try next one
			continue;
		}
		
		// check filename prefix
		if(strncmp(dirent->d_name, "hidraw", 6) != 0){
			// prefix not matching
			continue;
		}
		
		
		// CHECK IDS //
		// get full path by appending '/dev/'
		char path[PATH_MAX];
		snprintf(path, sizeof(path) - 1, "/dev/%s", dirent->d_name);
		
		// open hid device (readonly, non-blocking)
		int hid = open(path, O_RDONLY|O_NONBLOCK);
		if(hid < 0){
			// not able to open device, try next one
			continue;
		}
		
		// prepare report descriptor buffers
		struct hidraw_report_descriptor rpt_desc;
		memset(&rpt_desc, 0x00, sizeof(rpt_desc));
		
		struct hidraw_devinfo info;
		memset(&info, 0x00, sizeof(info));
		
		char buf[256];
		memset(buf, 0x00, sizeof(buf));
		
		// get size of report descriptor
		int res;
		int desc_size = 0;
		res = ioctl(hid, HIDIOCGRDESCSIZE, &desc_size);
		if(res < 0){
			// problem reading, try next one
			close(hid);
			continue;
		}
		
		// get report descriptor
		rpt_desc.size = desc_size;
		res = ioctl(hid, HIDIOCGRDESC, &rpt_desc);
		if(res < 0){
			// problem reading, try next one
			close(hid);
			continue;
		}
		
		// get ids (read info section of report descriptor)
		res = ioctl(hid, HIDIOCGRAWINFO, &info);
		if(res < 0){
			// problem reading, try next one
			close(hid);
			continue;
		}
		
		// compare ids
		if((uint16_t)info.vendor == search_vendor_id && (uint16_t)info.product == search_product_id){
			// found a device!
			// close dir
			closedir(dir);
			
			// return file handle
			return(hid);
		}
	}
	
	
	// DIDN'T FIND ANYTHING //
	// close dir
	closedir(dir);
	
	// error message
	fprintf(stderr, "[ERROR] No hid device found\n");
	exit(3);
}

/*
	HELPER: Find bus device file of form '/dev/bus/usb/xxx/xxx' for device with SEARCH-VENDOR-ID and SEARCH-PRODUCT-ID and return its handle
*/
int find_device_bus(uint16_t search_vendor_id, uint16_t search_product_id){
	// OPEN /SYS/BUS/USB/DEVICES AS DIR //
	DIR *dir = opendir("/sys/bus/usb/devices");
	struct dirent *dirent;
	
	
	// CHECK IF READABLE //
	if(dir == NULL){
		// not readable
		fprintf(stderr, "[ERROR] /sys/bus/usb/devices not readable\n");
		exit(0);
	}
	
	
	// check every file in dir
	char path[PATH_MAX];
	char id_buf[129];
	int file;
	uint16_t our_vendor_id;
	uint16_t our_product_id;
	int our_bus_num;
	int our_dev_num;
	while((dirent = readdir(dir)) != NULL){
		// ONLY TRY DIRS STARTING WITH DIGIT //
		if(!isdigit(dirent->d_name[0])){
			// try next device
			continue;
		}
		
		
		// GET VENDOR-ID //
		// get file path
		snprintf(path, sizeof(path) - 1, "/sys/bus/usb/devices/%s/idVendor", dirent->d_name);
		
		// open file
		file = open(path, O_RDONLY);
		if(file < 0){
			// not able to open file, try next device
			continue;
		}
		
		// read first 4 chars from file
		memset(id_buf, 0x00, sizeof(id_buf));
		read(file, id_buf, 4);
		close(file);
		
		// decode hex-string
		our_vendor_id = strtol(id_buf, NULL, 16);
		
		
		// GET PRODUCT-ID //
		// get file path
		snprintf(path, sizeof(path) - 1, "/sys/bus/usb/devices/%s/idProduct", dirent->d_name);
		
		// open file
		file = open(path, O_RDONLY);
		if(file < 0){
			// not able to open file, try next device
			continue;
		}
		
		// read first 4 chars from file
		memset(id_buf, 0x00, sizeof(id_buf));
		read(file, id_buf, 4);
		close(file);
		
		// decode hex-string
		our_product_id = strtol(id_buf, NULL, 16);
		
		
		// COMPARE IDS //
		if(our_vendor_id != search_vendor_id || our_product_id != search_product_id){
			// no match, try next device
			continue;
		}
		
		
		// GET BUS-NUM //
		// get file path
		snprintf(path, sizeof(path) - 1, "/sys/bus/usb/devices/%s/busnum", dirent->d_name);
		
		// open file
		file = open(path, O_RDONLY);
		if(file < 0){
			// not able to open file, try next device
			continue;
		}
		
		// read first 128 chars from file
		memset(id_buf, 0x00, sizeof(id_buf));
		read(file, id_buf, 128);
		close(file);
		
		// decode string
		our_bus_num = strtol(id_buf, NULL, 10);
		
		
		// GET DEV-NUM //
		// get file path
		snprintf(path, sizeof(path) - 1, "/sys/bus/usb/devices/%s/devnum", dirent->d_name);
		
		// open file
		file = open(path, O_RDONLY);
		if(file < 0){
			// not able to open file, try next device
			continue;
		}
		
		// read first 128 chars from file
		memset(id_buf, 0x00, sizeof(id_buf));
		read(file, id_buf, 128);
		close(file);
		
		// decode string
		our_dev_num = strtol(id_buf, NULL, 10);
		
		
		// GET BUS PATH //
		// close dir
		closedir(dir);
		
		// format as usb-bus
		char bus[PATH_MAX];
		snprintf(bus, sizeof(bus) - 1, "/dev/bus/usb/%03d/%03d", our_bus_num, our_dev_num);
		
		// return file handle
		return(open(bus, O_WRONLY));
	}
	
	
	// DIDN'T FIND ANYTHING //
	// close dir
	closedir(dir);
	
	// error message
	fprintf(stderr, "[ERROR] No bus device found\n");
	exit(3);
}

/*
	HELPER: Read hid data and output as json on stdout
*/
void output_data(){
	// FIND HID DEVICE //
	int hid = find_device_hid(vendor_id, product_id);
	
	
	// READ DATA //
	// charge and remaining time
	int values_res;
	char values_buffer[64];
	for(size_t q = 0; q <= 1; q++){
		values_buffer[0] = 0x06;
		values_res = ioctl(hid, HIDIOCGFEATURE(256), values_buffer);
		if(values_res >= 4){
			// success
			break;
		}
		
		// max tries exceeded?
		fprintf(stderr, "[WARN ] Unable to fetch report 0x06\n");
		if(q >= 1){
			close(hid);
			exit(2);
		}
		
		// maybe try a usb reset
		if(auto_usbreset){
			fprintf(stderr, "[INFO ] Trying usb reset\n");
			usbreset();
		}
	}
	
	// flaggs (charging, discharging, line power, low battery)
	int flaggs_res;
	char flaggs_buffer[64];
	for(size_t q = 0; q <= 1; q++){
		flaggs_buffer[0] = 0x01;
		flaggs_res = ioctl(hid, HIDIOCGFEATURE(256), flaggs_buffer);
		if(flaggs_res >= 5){
			// success
			break;
		}
		
		// max tries exceeded?
		fprintf(stderr, "[WARN ] Unable to fetch report 0x01\n");
		if(q >= 1){
			close(hid);
			exit(2);
		}
		
		// maybe try a usb reset
		if(auto_usbreset){
			fprintf(stderr, "[INFO ] Trying usb reset\n");
			usbreset();
		}
	}
	
	// close file
	close(hid);
	
	
	// GET DATA //
	// charge
	uint8_t charge = values_buffer[1];
	
	// runtime (little-endian)
	uint16_t runtime = values_buffer[2] + (values_buffer[3] << 8);
	
	// line powered?
	uint8_t line = flaggs_buffer[1];
	
	// battery low?
	uint8_t battery_low = flaggs_buffer[2];
	
	// charging?
	uint8_t charging = flaggs_buffer[3];
	
	// discharging?
	uint8_t discharging = flaggs_buffer[4];
	
	
	// OUTPUT AS JSON //
	printf("{\"charge\":%d,\"runtime\":%d,\"line\":%s,\"battery_low\":%s,\"charging\":%s,\"discharging\":%s}", charge, runtime, bool_string(line), bool_string(battery_low), bool_string(charging), bool_string(discharging));
}

/*
	HELPER: Do usb reset
*/
void usbreset(){
	// FIND BUS DEVICE //
	int bus = find_device_bus(vendor_id, product_id);
	
	
	// DO RESET //
	// send command
	if(ioctl(bus, USBDEVFS_RESET, 0) < 0){
        fprintf(stderr, "[WARN ] Unable to do usbreset\n");
		return;
    }
	
	// success!
    fprintf(stderr, "[INFO ] Usbreset successful\n");
	
	// close file
    close(bus);
}

/*
	HELPER: Print version and exit
*/
void print_version(){
	printf("phoenix-upshid v1.0.2-a | (c) DrMaxNix 2022 | www.drmaxnix.de/phoenix-upshid\n");
	exit(0);
}

/*
	HELPER: Print help message and exit
*/
void print_help(char *cmd){
	// usage
	printf("Usage:\n");
	printf("  %s [-a] <vendor-id>:<product-id>\n", cmd);
	printf("\n");
	
	// options
	printf("  -a               Automatically try usb reset on failed read\n");
	printf("  \n");
	printf("  -h               Show this help message and exit\n");
	printf("  -v               Print version and exit\n");
	printf("  \n");
	printf("  <vendor-id>      Vendor-id of the hid-device to search for\n");
	printf("  <product-id>     Product-id of the hid-device to search for\n");
	printf("  \n");
	printf("  <vendor-id> and <product-id> are 4-digit hexadecimal values (eg. '06da')\n");
	printf("\n");
	
	// example
	printf("Example: print device data of device 06da:ffff (and try usb reset)\n");
	printf("  %s -a 06da:ffff\n", cmd);
	printf("\n");
	
	// return values
	printf("Return values:\n");
	printf("  0: Success (valid data will be returned on stdout)\n");
	printf("  1: User input error\n");
	printf("  2: Ups state unknown\n");
	printf("  3: Ups is dead (\"not found\")\n");
	printf("\n");
	exit(0);
}

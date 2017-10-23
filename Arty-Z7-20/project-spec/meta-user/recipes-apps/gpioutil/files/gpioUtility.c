/*This GPIO utility checks for GPIO devices which exist
    in an FPGA and are declared to the kernel as uio devices */

#include <libgpio.h>
#include <libuio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>


void printDeviceInfo();
void printUsage(char * progName);
void parse_opts(int argc, char * argv[]);

short modeFlag = -1;

int main(int argc, char * argv[]) {
    parse_opts(argc, argv);
    int   uioNum = atoi(argv[3]), 
            mapNum = atoi(argv[4]), 
            channelNum = atoi(argv[5]),
            gpioNum = atoi(argv[2]);


    // printf("UIO device number: %d\n", uioNum);
    // printf("UIO map number: %d\n", mapNum);
    // printf("Channel number: %d\n", channelNum);
    // printf("GPIO number: %d\n", gpioNum);
    GPIO vm = GPIO_init(uioNum, mapNum);
    
    short writeVal;
    unsigned char readVal;
    if(modeFlag == OUTPUT) {
        writeVal = atoi(argv[6]);
        setPinMode(vm, channelNum, gpioNum + 1, modeFlag);
        digitalWrite(vm, channelNum, gpioNum + 1, writeVal);
    } else if(modeFlag == INPUT) {
        setPinMode(vm, channelNum, gpioNum + 1, modeFlag);
        readVal = digitalRead(vm, channelNum, gpioNum + 1);
        printf("%s\n", readVal ? "0x1" : "0x0");
    }

    GPIO_Close(vm);
}



/*************************************************************
 *
 *  Crawls the /sys/class/uio and /proc/device-tree/
 *  directories and prints out information about the valid
 *  uio devices that are gpio controllers
 *
 *************************************************************/
void printDeviceInfo() {
    printf("\n");
    short num_uio_devices = 0;
    struct dirent * info;
    
    DIR * uio_base = opendir("/sys/class/uio");
    
    
    if(NULL == uio_base) {
        perror("Could not open the uio base directory");
        exit(EXIT_FAILURE);
    }
    while(1) {
        info = readdir(uio_base);
        if(NULL == info) {
            break;
        }
        
        // Only dig deeper if we're not look at the parent or current directory
        if((strcmp("..", info->d_name) != 0) && (strcmp(".", info->d_name) != 0)) {
            
            char stub[100];
            sprintf(stub, "/sys/class/uio/%s/name", info->d_name);
            FILE * name = fopen(stub, "r");
            if(NULL == name) {
                fprintf(stderr, "Could not find 'name' field for this UIO device: %s\n", info->d_name);
                break;
            }

            // Check to see if the UIO device is a gpio controller
            char name_buf[5];
            fgets(name_buf, 5, name);
            fclose(name);

            if(!strcmp(name_buf, "gpio")) {
                printf("[------------------------%s--------------------\n", info->d_name);
                // if(info->d_name[3] - '0' > num_uio_devices) {
                //     num_uio_devices = info->d_name[3] - '0';
                // }

                num_uio_devices++;
                char uio_maps[100];
                sprintf(uio_maps, "/sys/class/uio/uio%d/maps", info->d_name[3] - '0');
                DIR * map_base = opendir(uio_maps);
                struct dirent * map_info;
                short numMaps = -1;
                while(1) {
                    map_info = readdir(map_base);
                    if(NULL == map_info) {
                        break;
                    }

                    // No need to run analysis on the parent or current directory
                    if((strcmp("..", map_info->d_name) != 0) && (strcmp(".", map_info->d_name) != 0)) {
                        if(map_info->d_name[3] - '0' > numMaps) {
                            numMaps = map_info->d_name[3] - '0';
                        }
                        
                        printf("[\t-------------------%s-----------------\n", map_info->d_name);
                        char part_info[100];
                        sprintf(part_info, "/sys/class/uio/uio%d/maps/map%d", info->d_name[3] - '0', map_info->d_name[3] - '0');
                        
                        // Print name and address information about the map
                        char address_path[100], name_path[100], map_adx[50], map_name[50];
                        sprintf(address_path, "%s/addr", part_info);
                        sprintf(name_path, "%s/name", part_info);
                        FILE * adx = fopen(address_path, "r");
                        FILE * name = fopen(name_path, "r");
                        if(NULL == adx || NULL == name) {
                            perror("Error opening the name or address file for UIO");
                        }
                        fgets(map_adx, 50, adx);
                        fgets(map_name, 50, name);
                        printf("[\t\tAddress: %s", map_adx);
                        printf("[\t\tName: %s", map_name);
                        fclose(adx);                        
                        fclose(name);
                        printf("[\t\tMap Number: %d\n", map_info->d_name[3] - '0');

                        char * index_of = strchr(map_name, '\n');
                        *index_of = '\0';

                        // Use the name information to check out the device-tree entry and get
                        // Information about the dimensions and specifications
                        // Of the gpio controller
                        char dtree_base[100];
                        sprintf(dtree_base, "/proc/device-tree%s", map_name);

                        // Get the number of channels
                        char channel_path[100], channel_buf[5];
                        sprintf(channel_path, "%s/xlnx,is-dual", dtree_base);
                        FILE * fchannel = fopen(channel_path, "r");
                        if(NULL == fchannel) {
                            perror("Could not open the channel file");
                        }
                        fgets(channel_buf, 5, fchannel);

                        printf("[\t\tNumber of Channels: %d\n", channel_buf[3] ? 2 : 1);

                        char chan1_path[100], chan1_width[5];
                        sprintf(chan1_path, "%s/xlnx,gpio-width", dtree_base);
                        FILE * fchan1 = fopen(chan1_path, "r");
                        fgets(chan1_width, 5, fchan1);
                        fclose(fchan1);
                        printf("[\t\tChannel 1 Width: %d\n", chan1_width[3]);

                        if(channel_buf[3]) {
                            char chan2_path[100], chan2_width[5];
                            sprintf(chan2_path, "%s/xlnx,gpio2-width", dtree_base);
                            FILE * fchan2 = fopen(chan2_path, "r");
                            fgets(chan2_width, 5, fchan2);
                            fclose(fchan2);
                            printf("[\t\tChannel 2 Width: %d\n", chan2_width[3]);
                        }
                    }

                    
                }
                printf("[\tNumber of Maps: %d\n", numMaps + 1);
                printf("[UIO Device Number: %d\n\n", info->d_name[3] - '0');
                closedir(map_base);
            }
        }
        
    }
    printf("Number of GPIO devices: %d\n", num_uio_devices);
    closedir(uio_base);
    exit(EXIT_SUCCESS);
}

/*************************************************************
 *
 *  Prints the command-line argument usage specifications
 *
 *************************************************************/
void printUsage(char * progName) {
    printf("Usage: %s [UIO Device Number] [Map Number] [Channel Number (1 or 2)] [-i -o -h -d] [GPIO Number] [Value when using -o -- '0' or '1']\n", progName);
	puts("  -i --input    Read from the GPIO specified by GPIO Number\n"
	     "  -o --output   Write 'Value'to the GPIO specified by GPIO Number\n"
         "  -h --help     Print this usage information\n"
         "  -d --device   Print information about the UIO GPIO controllers present\n\n\n");
    printf("To write to a GPIO: '%s 0 0 1 -o 4 1' will set gpio #4 of channel 1 of uio device 0 to HIGH\n", progName);
    printf("To read from a GPIO: '%s 0 0 1 -i 4' will read the current value of gpio #4 of channel 1 of uio device 0\n", progName);
	exit(1);
}

void parse_opts(int argc, char * argv[]) {
    while(1) {
        static const struct option lopts[] = {
            {"input", 0, 0, 'i'},
            {"output", 0, 0, 'o'},
            {"help", 0, 0, 'h'},
            {"device", 0, 0, 'd'},
            {NULL, 0, 0, 0},
        };

        int c;

        c = getopt_long(argc, argv, "i:o:h:d", lopts, NULL);
        if(-1 == c) {
            break;
        }

        switch(c) {
            case 'i':
                if(modeFlag == INPUT || modeFlag == OUTPUT) {
                    printUsage(argv[0]);
                    break;
                }
                modeFlag = INPUT;
                break;
            case 'o':
                if(modeFlag == INPUT || modeFlag == OUTPUT) {
                    printUsage(argv[0]);
                    break;
                }
                modeFlag = OUTPUT;
                break;
            case 'h':
                printUsage(argv[0]);
                break;
            case 'd':
                printDeviceInfo();
                break;
            default:
                printUsage(argv[0]);
                break;
        }
    }
}
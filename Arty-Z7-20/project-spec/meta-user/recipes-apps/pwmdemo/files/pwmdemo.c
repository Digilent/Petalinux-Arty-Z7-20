#include <libpwm.h>
#include <libuio.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>

void getDeviceInfo(uint8_t * uioNum, uint8_t * mapNum);
void exitHandler();

#define ZERO    0x0000
#define TWENTYFIVE 0x0800
#define FIFTY   0x8000

PWM * pwm;

int main(int argc, char * argv[]) {
    uint16_t r1 = 0, r2 = 0, 
            b1 = 0, b2 = 0, 
            g1 = 0, g2 = 0;
    
    uint8_t b1flag = 0, b2flag = 0;

    uint8_t uioNum, mapNum;
    getDeviceInfo(&uioNum, &mapNum);

    printf("Using PWM device at UIO:%d, MAP:%d\n", uioNum, mapNum);
    printf("Beginning PWM demo....\n");
    printf("Press ctrl+c to exit\n");

    struct sigaction sigIntHandler;
    
    sigIntHandler.sa_handler = exitHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    
    sigaction(SIGINT, &sigIntHandler, NULL);


    pwm = PWM_init(uioNum, mapNum);
    PWM_Enable(pwm);

    // PL Clock is 100 MHz, each successive value in the frequency register is 10ns
    // Set PWM frequency to 10 * 100000 ==  1000000 ns     ==     1 KHz
    setPwmPeriod(pwm, 59999);
    uint16_t i = 0;
    while(1) {

        // setPwmDuty(pwm, 1, b1);
        setPwmDuty(pwm, 2, g1);
        setPwmDuty(pwm, 3, r1);
        setPwmDuty(pwm, 4, b2);
        setPwmDuty(pwm, 5, g2);
        setPwmDuty(pwm, 6, r2);
        
        if(i == 0) {
            b1 = 19998;
            r1 = 0;
            g1 = 0;
            g2 = 19998;
            r2 = 0;
            b2 = 0;
        } else if(i < 20000) {
            setPwmDuty(pwm, 1, b1--);
            setPwmDuty(pwm, 3, r1++);
            
            setPwmDuty(pwm, 5, (g2 == 0) ? 0 : g2--);
            setPwmDuty(pwm, 6, r2++);
            
        } else if (i < 40000) {
            setPwmDuty(pwm, 3, (r1 == 0) ? 0 : r1--);
            setPwmDuty(pwm, 2, g1++);

            setPwmDuty(pwm, 4, b2++);
            setPwmDuty(pwm, 6, (r2 == 0) ? 0 : r2--);
        } else if (i < 60000) {
            setPwmDuty(pwm, 2, (g1 == 0) ? 0 : g1--);
            setPwmDuty(pwm, 1, b1++);

            setPwmDuty(pwm, 4, (b2 == 0) ? 0 : b2--);
            setPwmDuty(pwm, 5, g2++);

        }
 
        (i == 59999) ? i = 0 : i++;
        usleep(150);
    }



}

void getDeviceInfo(uint8_t * uioNum, uint8_t * mapNum) {
    printf("\n");
    short num_pwm_devices = 0;
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

        if((strcmp("..", info->d_name) != 0) && (strcmp(".", info->d_name) != 0)) {
            char stub[100];
            sprintf(stub, "/sys/class/uio/%s/name",  info->d_name);


            FILE * name = fopen(stub, "r");
            if(NULL == name) {
                fprintf(stderr, "Could not find the name for for this UIO device: %s\n", info->d_name);
                break;
            }

            char name_buf[4];
            fgets(name_buf, 4, name);
            fclose(name);

            // Check to see if 'NAME' matches PWM
            if(!strcmp(name_buf, "PWM")) {
                printf("[------------------------%s--------------------\n", info->d_name);
                *uioNum = info->d_name[3] - '0';
                num_pwm_devices++;

                char uio_maps[100];
                sprintf(uio_maps, "/sys/class/uio/uio%d/maps", info->d_name[3] - '0');
                DIR * map_base = opendir(uio_maps);
                
                struct dirent * map_info;
                short numMaps = 0;
                while(1) {
                    map_info = readdir(map_base);

                    if(NULL == map_info) {
                        break;
                    }

                    if((strcmp("..", map_info->d_name) != 0) && (strcmp(".", map_info->d_name) != 0)) {
                        numMaps++;
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
                        *mapNum = map_info->d_name[3] - '0';
                        printf("[\t\tMap Number: %d\n", map_info->d_name[3] - '0');
                    }
                }
                printf("[\tNumber of Maps: %d\n", numMaps);
                printf("[UIO Device Number: %d\n\n", info->d_name[3] - '0');
                closedir(map_base);
                
            }

        }

    }
    printf("Number of PWM devices: %d\n", num_pwm_devices);
    closedir(uio_base);
}

void exitHandler() {
    PWM_Disable(pwm);
    PWM_Close(pwm);
    printf("\nExiting PWM demo\n");
    exit(EXIT_SUCCESS);

}
#include <libvrc.h>
#include <getopt.h>

struct drm_mode_modeinfo printDeviceInfo(char * argv[]);
void parse_opts(int argc, char * argv[]);
void printUsage(char * progName);

int8_t devNum = -1;

int main(int argc, char * argv[]) {
    parse_opts(argc, argv);
    struct drm_mode_modeinfo drmMode = printDeviceInfo(argv);

    drm_cntrl * test;
    test = drmControlInit("/dev/dri/card0", drmMode);
    
    uint32_t currentSize = test->create_dumb[test->current_fb]->size;
    uint8_t * mem = (uint8_t *)test->fbMem[0];
    for(int i = 0; i < currentSize; i += 3) {
        *(mem + i) = i;
        *(mem + i + 1) = i;
        *(mem + i + 2) = i;
    }

    char c;
    while(1) {
        c = fgetc(stdin);
        if(c == '^C') {
            break;
        }
    }

    drmControlClose(test);
    return EXIT_SUCCESS;

}

struct drm_mode_modeinfo printDeviceInfo(char * argv[]) {
    int dri_fd  = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if(dri_fd < 0) {
        perror("Unable to open the requested port");
        exit(EXIT_FAILURE);
    }

    struct drm_mode_card_res * mode_card = (struct drm_mode_card_res *) malloc(sizeof(struct drm_mode_card_res));
    
    if(ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, mode_card) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Unable to get resources -- 1st pass\n");
        exit(EXIT_FAILURE);
    }

    uint64_t res_fb_buf[10] = {0};
    uint64_t res_crtc_buf[10] = {0};
    uint64_t res_conn_buf[10] = {0};
    uint64_t res_enc_buf[10] = {0};

    (*mode_card).fb_id_ptr = (uint64_t) res_fb_buf;
    (*mode_card).crtc_id_ptr = (uint64_t) res_crtc_buf;
    (*mode_card).connector_id_ptr = (uint64_t) res_conn_buf;
    (*mode_card).encoder_id_ptr = (uint64_t) res_enc_buf;

    if(ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, mode_card) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Unable to get resources -- 2nd pass\n");
        exit(EXIT_FAILURE);
    }
    
    struct drm_mode_modeinfo mode_buffer[20] = {0};
    uint64_t conn_prop_buf[20] = {0};
    uint64_t conn_propval_buf[20] = {0};
    uint64_t conn_enc_buf[20] = {0};

    struct drm_mode_get_connector * connector = (struct drm_mode_get_connector *) malloc(sizeof(struct drm_mode_get_connector));
    
    (*connector).connector_id = res_conn_buf[0];

    if(ioctl(dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, connector) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Unable to get connector -- 1st pass\n");
        exit(EXIT_FAILURE);
    }
    
    (*connector).modes_ptr = (uint64_t) mode_buffer;
    (*connector).props_ptr = (uint64_t) conn_prop_buf;
    (*connector).prop_values_ptr = (uint64_t) conn_propval_buf;
    (*connector).encoders_ptr = (uint64_t) conn_enc_buf;

    if(ioctl(dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, connector) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Unable to get connector -- 2nd pass\n");
        exit(EXIT_FAILURE);
    }
    
    /* Check if this connector is currently connected */
    if((*connector).count_encoders < 1 || (*connector).count_modes < 1 || !((*connector).encoder_id 
            || !((*connector).connection))) {
                fprintf(stderr, "Not Connected\n");
    }
    

    struct drm_mode_modeinfo drmMode;
    /* Print Information about the available modes */
    if(strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--device") == 0) {
        for(int i = 0; i < (*connector).count_modes; i++) {
            struct drm_mode_modeinfo * modePtr = (struct drm_mode_modeinfo *) (*connector).modes_ptr + i;
            fprintf(stderr, "***********************************************\n");
            fprintf(stderr, "[Resolution Mode: %d\n", i);
            fprintf(stderr, "[\t%s\n", (*modePtr).name);
            fprintf(stderr, "[\tClock: %d", (*modePtr).clock);
            fprintf(stderr, "\t\tVrefresh: %d\n", (*modePtr).vrefresh);
            fprintf(stderr, "***********************************************\n\n\n");
        }
        free(connector);
        free(mode_card);
        close(dri_fd);
        exit(EXIT_SUCCESS);
    } else {
        struct drm_mode_modeinfo * modePtr = (struct drm_mode_modeinfo *) (*connector).modes_ptr + devNum;
        drmMode = *modePtr;
        free(connector);
        free(mode_card);
        close(dri_fd);
    }

    return drmMode;
}

void parse_opts(int argc, char * argv[]) {
    while(1) {
        static const struct option lopts[] = {
            {"set", 0, 0, 's'},
            {"device", 0, 0, 'd'},
            {"help", 0, 0, 'h'},
            {NULL, 0, 0, 0},
        };

        int c;

        c = getopt_long(argc, argv, "s:h:d", lopts, NULL);
        if(-1 == c) {
            break;
        }

        switch(c) {
            case 's':
                devNum = atoi(argv[2]);
                break;
            case 'h':
                printUsage(argv[0]);
                break;
            case 'd':
                printDeviceInfo(argv);
                break;
            default:
                printUsage(argv[0]);
                break;
        }
    }
}

void printUsage(char * progName) {
    printf("Usage: %s [ -d | -s | -h ] [Resolution Mode # to select (if using -s)] \n", progName);
	puts("  This program will use the HDMI output to gather the valid resolution modes from the connected monitor\n");
    puts("  using the '-d' option will print these valid resolution modes to the console\n");
    puts("  Additionally, this program will draw some basic shapes to the connected monitor\n\n");
    puts("  -h --help     Print this usage information\n"
         "  -d --device   Print information about the supported resolution modes\n"
         "  -s --set      Set the connected monitor to the resolution mode specific by the flag\n\n");
    printf("  To get information about supported resolutions, type '%s -d'\n", progName);
    printf("  To set a resolution, type '%s -s [n]', where [n] is the listed resolution mode number given by using -d\n", progName);
    printf("  This utility defaults to port /dev/dri/card0\n\n");
	exit(EXIT_SUCCESS);
}
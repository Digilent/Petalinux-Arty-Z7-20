#include <libvrc.h>
/*
 *  Allocates space for a 'drm_control' struct and returns a pointer to it
 *  Sets the CRTC to the resolution requested
 *  Creates 4 dumb buffers with the requested resolutions and maps them into virtual memory
 *  populates the drm_control structure with all the necessary information
 *  to control linking the different dumb buffers to the CRTC  
 *  Initially, dumb buffer 1 is mapped to the CRTC
 *
 *  returns 0 on success
 *  returns -1 on failure
 */
 drm_cntrl * drmControlInit(char * port, struct drm_mode_modeinfo mode) {
    int dri_fd;
    if(openPort(port, &dri_fd) < 0) {
        fprintf(stderr, "Failed to open the file specified by 'port' %m\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for our main structure that will be returned and which contains
    // All the necessary information
    drm_cntrl * control = (drm_cntrl *) malloc(sizeof(drm_cntrl));
    
    // Allocate memory for the current mode
    control->current = (struct drm_mode_modeinfo *) malloc(sizeof(struct drm_mode_modeinfo));
    
    // Populate fields of the main data structure
    *(control->current) = mode;    // Set the current mode to the one passed by the user
    control->current_fb = 0;        // Initially link FB0 to the CRTC
    control->dri_fd = dri_fd;

    /* Take control of the DRM MASTER */
    if(ioctl(control->dri_fd, DRM_IOCTL_SET_MASTER, 0) < 0) {
        fprintf(stderr, "%s         ", strerror(errno));
        fprintf(stderr, "Unable to Set Master\n");
        exit(EXIT_FAILURE);
    }

    /*****************************************************************/
    /* Get Resource Statistics */
    /*****************************************************************/
    
    // Allocate memory for resolution information
    control->mode_card = (struct drm_mode_card_res *) malloc(sizeof(struct drm_mode_card_res));

    if(ioctl(control->dri_fd, DRM_IOCTL_MODE_GETRESOURCES, control->mode_card) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Unable to get resources -- 1st pass\n");
        exit(EXIT_FAILURE);
    }

    /* These fields will be populated by the next ioctl call */
    uint64_t res_fb_buf[10] = {0};
    uint64_t res_crtc_buf[10] = {0};
    uint64_t res_conn_buf[10] = {0};
    uint64_t res_enc_buf[10] = {0};
    control->mode_card->fb_id_ptr = (uint64_t) res_fb_buf;
    control->mode_card->crtc_id_ptr = (uint64_t) res_crtc_buf;
    control->mode_card->connector_id_ptr = (uint64_t) res_conn_buf;
    control->mode_card->encoder_id_ptr = (uint64_t) res_enc_buf;
    
    if(ioctl(control->dri_fd, DRM_IOCTL_MODE_GETRESOURCES, control->mode_card) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Unable to get resources -- 2nd pass\n");
        exit(EXIT_FAILURE);
    }
    /*****************************************************************/


    /***************************************************************
    *   Get information about the connector and available modes
    ***************************************************************/
    // Allocate memory for the connector
    control->connector = (struct drm_mode_get_connector *) malloc(sizeof(struct drm_mode_get_connector));
    
    struct drm_mode_modeinfo mode_buffer[20] = {0};     // Will populate with the available modes given by the monitor
    uint64_t conn_prop_buf[20] = {0};       
    uint64_t conn_propval_buf[20] = {0};
    uint64_t conn_enc_buf[20] = {0};

    control->connector->connector_id = res_conn_buf[0];
    if(ioctl(dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, control->connector) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Unable to get connector -- 1st pass\n");
        exit(EXIT_FAILURE);
    }

    control->connector->modes_ptr = (uint64_t) mode_buffer;
    control->connector->props_ptr = (uint64_t) conn_prop_buf;
    control->connector->prop_values_ptr = (uint64_t) conn_propval_buf;
    control->connector->encoders_ptr = (uint64_t) conn_enc_buf;
    if(ioctl(dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, control->connector) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Unable to get connector -- 2nd pass\n");
        exit(EXIT_FAILURE);
    }

    /* Check if this connector is currently connected */
    if(control->connector->count_encoders < 1 || control->connector->count_modes < 1 || !(control->connector->encoder_id) 
            || !(control->connector->connection)) {
                fprintf(stderr, "Connector Not Connected\n");
                exit(EXIT_FAILURE);
    }



    /***************************************************************
    *   Create FOUR dumb buffers
    ***************************************************************/
    for(int i = 0; i < NUM_DUMB_BUFFERS; i++) {
        /* Allocate all of the memory for the frame buffer structs, and assign
            the pointers into our drm_cntrl struct                              */
        control->create_dumb[i] = (struct drm_mode_create_dumb *) malloc(sizeof(struct drm_mode_create_dumb));
        control->map_dumb[i] = (struct drm_mode_map_dumb *) malloc(sizeof(struct drm_mode_map_dumb));
        control->cmd_dumb[i] = (struct drm_mode_fb_cmd *) malloc(sizeof(struct drm_mode_fb_cmd));

        control->create_dumb[i]->width = control->current->hdisplay;
        control->create_dumb[i]->height = control->current->vdisplay;
        control->create_dumb[i]->bpp = 24;
        control->create_dumb[i]->flags = 0;
        control->create_dumb[i]->pitch = 0;
        control->create_dumb[i]->size = 0;
        control->create_dumb[i]->handle = 0;
        if(ioctl(control->dri_fd, DRM_IOCTL_MODE_CREATE_DUMB, control->create_dumb[i]) < 0) {
            fprintf(stderr, "%d %s         ", errno, strerror(errno));
            fprintf(stderr, "Unable to create dumb buffer\n");
            exit(EXIT_FAILURE);
        }
    
        control->cmd_dumb[i]->width = control->create_dumb[i]->width;
        control->cmd_dumb[i]->height = control->create_dumb[i]->height;
        control->cmd_dumb[i]->bpp = control->create_dumb[i]->bpp;
        control->cmd_dumb[i]->pitch = control->create_dumb[i]->pitch;
        control->cmd_dumb[i]->depth = 32;
        control->cmd_dumb[i]->handle = control->create_dumb[i]->handle;
        if(ioctl(control->dri_fd, DRM_IOCTL_MODE_ADDFB, control->cmd_dumb[i]) < 0) {
            fprintf(stderr, "%d %s         ", errno, strerror(errno));
            fprintf(stderr, "Unable add dumb buffer as FB\n");
            exit(EXIT_FAILURE);
        }
    
        control->map_dumb[i]->handle = control->create_dumb[i]->handle;
        if(ioctl(control->dri_fd, DRM_IOCTL_MODE_MAP_DUMB, control->map_dumb[i]) < 0) {
            fprintf(stderr, "%d %s         ", errno, strerror(errno));
            fprintf(stderr, "Unable to map dumb buffer\n");
            exit(EXIT_FAILURE);
        }
    
        control->fbMem[i] = mmap(NULL, control->create_dumb[i]->size, PROT_READ | PROT_WRITE, MAP_SHARED, control->dri_fd, control->map_dumb[i]->offset);
        if(control->fbMem[i] == NULL || control->fbMem[i] == -1) {
            fprintf(stderr, "%d %s         ", errno, strerror(errno));
            fprintf(stderr, "Failed to mmap the FB memory\n");
            exit(EXIT_FAILURE);
        }

        control->fb_id[i] = control->cmd_dumb[i]->fb_id;
    }


    /***************************************************************
    *   Handle Encoder and CRTC
    ***************************************************************/
    // Allocate memory for the encoder information
    control->enc = (struct drm_mode_get_encoder *) malloc(sizeof(struct drm_mode_get_encoder));
    
    control->enc->encoder_id = control->connector->encoder_id;
    if(ioctl(control->dri_fd, DRM_IOCTL_MODE_GETENCODER, control->enc) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Failed to get the encoder\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the CRTC information
    control->crtc = (struct drm_mode_crtc *) malloc(sizeof(struct drm_mode_crtc));

    control->crtc->crtc_id = control->enc->crtc_id;
    if(ioctl(control->dri_fd, DRM_IOCTL_MODE_GETCRTC, control->crtc) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Failed to get the CRTC\n");
        exit(EXIT_FAILURE);
    }

    control->crtc->fb_id = control->fb_id[control->current_fb];
    control->crtc->set_connectors_ptr = (uint64_t) res_conn_buf;
    control->crtc->count_connectors = 1;
    
    control->crtc->mode = *(control->current); // Set to the requested mode
    control->crtc->mode_valid = 1;

    if(ioctl(control->dri_fd, DRM_IOCTL_MODE_SETCRTC, control->crtc) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Failed to set the CRTC\n");
        exit(EXIT_FAILURE);
    }

    return control;
}

/*
 *  Flips a new frame buffer onto the CRTC after the most
 *  recent vertical sync.
 *  The frame buffer to flip to is reference by int fb_num
 *  and represents the index of the frame buffer to use
 * 
 *  returns 0 on success
 *  returns -1 on failure
 */
int mapFBtoCRTC(drm_cntrl * control, int fb_num) {

    struct drm_mode_crtc_page_flip page_flip = {0};

    page_flip.crtc_id = control->crtc->crtc_id;
    page_flip.fb_id = control->fb_id[fb_num];
    if(ioctl(control->dri_fd, DRM_IOCTL_MODE_PAGE_FLIP, &page_flip) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Failed to flip the page\n");
        exit(EXIT_FAILURE);
    }
    control->current_fb = fb_num;

    return SUCCESS;
}


/*
 *  Opens the requested DRI port from /dev/dri/card*
 *  Places the file descriptor at the location of fd
 * 
 *  returns 0 on success
 *  returns -1 on failure
 */
static int openPort(char * port, int * fd) {
    int dri_fd = open(port, O_RDWR, O_CLOEXEC);
    if(dri_fd == -1) {
        return FAILURE;
    }

    *fd = dri_fd;
    return SUCCESS;
}

/*
 *  Copies the frame buffer contents of fb_num_src to fb_num_dst
 *  This is a memory copy function, not a page flip
 * 
 *  returns 0 on success
 */
int swapFB(drm_cntrl * control, int fb_num_dst, int fb_num_src) {
    uint8_t * fbMem_dst = (uint8_t *) control->fbMem[fb_num_dst];
    uint8_t * fbMem_src = (uint8_t *) control->fbMem[fb_num_src];
    for(int i = 0; i < control->create_dumb[fb_num_dst]->size; i++) {
        *(fbMem_dst + i) = *(fbMem_src + i);
    }
    return SUCCESS;
}

/*
 *  Closes all open file descriptors and frees all allocated memory
 * 
 *  returns 0 on success
 *  returns -1 on failure
 */
int drmControlClose(drm_cntrl * control) {
    for(int i = 0; i < NUM_DUMB_BUFFERS; i++) {
        free(control->map_dumb[i]);
        free(control->create_dumb[i]);
        free(control->cmd_dumb[i]);
        munmap(control->fbMem[i], control->create_dumb[i]->size);
    }

    free(control->current);
    free(control->mode_card);
    free(control->connector);
    free(control->enc);
    free(control->crtc);

    if(ioctl(control->dri_fd, DRM_IOCTL_DROP_MASTER, 0) < 0) {
        fprintf(stderr, "%d %s         ", errno, strerror(errno));
        fprintf(stderr, "Failed to drop the master\n");
        exit(EXIT_FAILURE);
    }

    close(control->dri_fd);
    free(control);
    return SUCCESS;
}

/*
 *  Creates a new resolution mode on the connector and CRTC
 *  Creates two new frame buffers to be used at the new resolution mode
 *  Populates the new data into the drm_cntrl structure
 * 
 *  returns 0 on success
 *  returns -1 on failure
 */
int setNewResolution(drm_cntrl * control, struct drm_mode_modeinfo mode) {
    drmControlClose(control);
    
    drm_cntrl * temp = drmControlInit("/dev/dri/card0", mode);
    *control = *temp;

    return SUCCESS;
}

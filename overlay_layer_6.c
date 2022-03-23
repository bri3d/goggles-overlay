#include "goggles_overlay.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define PLANE_ID 6

duss_result_t pop_func(duss_disp_instance_handle_t *disp_handle,duss_disp_plane_id_t plane_id, duss_frame_buffer_t *frame_buffer,void *user_ctx) {
    return 0;
}

int main() {
    duss_disp_plane_id_t plane_id = PLANE_ID;
    duss_hal_obj_handle_t disp_handle;
    duss_hal_obj_handle_t ion_handle;
    duss_disp_vop_id_t vop_id;
    uint32_t hal_device_open_unk = 0;
    duss_hal_mem_handle_t ion_buf_0;
    duss_hal_mem_handle_t ion_buf_1;
    void * fb0_virtual_addr;
    void * fb0_physical_addr;
    void * fb1_virtual_addr;
    void * fb1_physical_addr;
    duss_result_t res = 0;
    duss_disp_instance_handle_t *disp_instance_handle = (duss_disp_instance_handle_t *)calloc(1, sizeof(duss_disp_instance_handle_t));
    duss_frame_buffer_t *fb_0 = (duss_frame_buffer_t *)calloc(1,sizeof(duss_frame_buffer_t));
    duss_frame_buffer_t *fb_1 = (duss_frame_buffer_t *)calloc(1,sizeof(duss_frame_buffer_t));
    duss_disp_plane_blending_t *pb_0 = (duss_disp_plane_blending_t *)calloc(1, sizeof(duss_disp_plane_blending_t));
    
    duss_hal_device_desc_t device_descs[3] = {
        {"/dev/dji_display", &duss_hal_attach_disp, &duss_hal_detach_disp, 0x0},
        {"/dev/ion", &duss_hal_attach_ion_mem, &duss_hal_detach_ion_mem, 0x0},
        {0,0,0,0}
    };

    duss_hal_initialize(device_descs);
    
    res = duss_hal_device_open("/dev/dji_display",&hal_device_open_unk,&disp_handle);
    if (res != 0) {
        printf("failed to open dji_display device");
        exit(0);
    }
    res = duss_hal_display_open(disp_handle, &disp_instance_handle, 0);
    if (res != 0) {
        printf("failed to open display hal");
        exit(0);
    }
    res = duss_hal_display_reset(disp_instance_handle);
    if (res != 0) {
        printf("failed to reset display");
        exit(0);
    }
    res = duss_hal_display_aquire_plane(disp_instance_handle,0,&plane_id);
    if (res != 0) {
        printf("failed to acquire plane");
        exit(0);
    }
    res = duss_hal_display_register_frame_cycle_callback(disp_instance_handle, plane_id, &pop_func, 0);
    if (res != 0) {
        printf("failed to register callback");
        exit(0);
    }
    res = duss_hal_display_port_enable(disp_instance_handle, 3, 1);
    if (res != 0) {
        printf("failed to enable display port");
        exit(0);
    }
    
    // PLANE BLENDING

    pb_0->voffset = 575; // this is either 215 or 575 depending on hwid?
    pb_0->hoffset = 0;
    pb_0->order = 2;
    pb_0->is_enable = 1;
    pb_0->glb_alpha_en = 0;
    pb_0->glb_alpha_val = 0;
    pb_0->blending_alg = 1;

    res = duss_hal_display_plane_blending_set(disp_instance_handle, plane_id, pb_0);
    
    if (res != 0) {
        printf("failed to set blending");
        exit(0);
    }
    res = duss_hal_device_open("/dev/ion", &hal_device_open_unk, &ion_handle);
    if (res != 0) {
        printf("failed to open shared VRAM");
        exit(0);
    }
    res = duss_hal_device_start(ion_handle,0);
    if (res != 0) {
        printf("failed to start VRAM device");
        exit(0);
    }

    res = duss_hal_mem_alloc(ion_handle,&ion_buf_0,0x473100,0x400,0,0x17);
    if (res != 0) {
        printf("failed to allocate VRAM");
        exit(0);
    }
    res = duss_hal_mem_map(ion_buf_0,&fb0_virtual_addr);
    if (res != 0) {
        printf("failed to map VRAM");
        exit(0);
    }
    res = duss_hal_mem_get_phys_addr(ion_buf_0, &fb0_physical_addr);
    if (res != 0) {
        printf("failed ot get phys addr");
        exit(0);
    }
    printf("first buffer VRAM mapped virtual memory is at %p : %p\n", fb0_virtual_addr, fb0_physical_addr);

    res = duss_hal_mem_alloc(ion_handle,&ion_buf_1,0x473100,0x400,0,0x17);
    if (res != 0) {
        printf("failed to allocate VRAM");
        exit(0);
    }
    res = duss_hal_mem_map(ion_buf_1,&fb1_virtual_addr);
    if (res != 0) {
        printf("failed to map VRAM");
        exit(0);
    }
    res = duss_hal_mem_get_phys_addr(ion_buf_1, &fb1_physical_addr);
    if (res != 0) {
        printf("failed ot get phys addr");
        exit(0);
    }
    printf("second buffer VRAM mapped virtual memory is at %p : %p\n", fb1_virtual_addr, fb1_physical_addr);

    for(int j = 0; j < 1140; j++) {   
        uint8_t which_buf = j % 2;
        void *fb_addr = which_buf ? fb1_virtual_addr: fb0_virtual_addr;
        memset(fb_addr, 0, 0x473100);
        uint32_t y, x, location;
        for (y = 0; y < 300; y++) {
            for (x = 0; x < 300; x++) {

                location = (x + j) * (4) +
                        (y + 200) * 0x1680;
            // boring gradient with some opacity
            *((char *)fb_addr + location) = 100;        
            *((char *)fb_addr + location + 1) = 15+(x)/2;    
            *((char *)fb_addr + location + 2) = 200-(y)/5;   
            *((char *)fb_addr + location + 3) = 100;    
            }
        }
    
        duss_hal_mem_sync(which_buf ? ion_buf_1 : ion_buf_0, 1);
        
        duss_frame_buffer_t *fb = which_buf ? fb_1 : fb_0;
        fb->pixel_format = DUSS_PIXFMT_RGBA8888;
        fb->buffer = which_buf ? ion_buf_1: ion_buf_0;
        fb->frame_id = which_buf;
        fb->planes[0].bytes_per_line = 0x1680;
        fb->planes[0].offset = 0;
        fb->planes[0].plane_height = 810;
        fb->planes[0].bytes_written = 0x473100;
        fb->width = 1440;
        fb->height = 810;
        fb->plane_count = 1;

        res = duss_hal_display_push_frame(disp_instance_handle, plane_id, which_buf ? fb_1: fb_0);
        if (res != 0) {
            printf("failed to push frame");
            exit(0);
        }
        usleep(1000);
    }
    duss_hal_mem_free(ion_buf_0);
    duss_hal_mem_free(ion_buf_1);
    duss_hal_display_release_plane(disp_instance_handle, plane_id);
    duss_hal_device_close(disp_handle);
    duss_hal_device_stop(ion_handle);
    duss_hal_device_close(ion_handle);
    duss_hal_deinitialize();
    free(fb_0);
    free(fb_1);
    free(pb_0);
    free(disp_instance_handle);
}
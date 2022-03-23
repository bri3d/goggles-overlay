# Raw Framebuffer Overlay for DJI Goggles 

For use with DJI Goggles V1 rooted with https://github.com/fpv-wtf/margerine .

See architecture primer at https://github.com/fpv-wtf/margerine/wiki .

This uses the DJI HAL layer to create a new framebuffer graphics layer, allocate shared "VRAM", and blit data to the screen using the extremely basic 2D layer acceleration present on the DJI Goggles CPU. 

Unfortunately it appears the dji_display "ICC" (Inter Core Communication) device cannot be shared, so we'll need to pull a pointer to the display object out of a running process to add another layer over the top of it.

Build using the Android NDK, like: `armv7a-linux-androideabi19-clang overlay_layer_6.c -L/dumped_dji_goggles_firmware/system/lib -lduml_hal -o overlay_layer_6 -O3`

I implemented double buffering even though it's unnecessary in the current architecture due to the memory barrier. The idea is that ideally we would spawn a task/thread which would manipulate the second buffer while waiting for the memory barrier on the first buffer to complete. 
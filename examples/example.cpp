#include "RaspiVid.h"
#include <memory>

using namespace std;
using namespace raspivid;

#define     RESIZE_WIDTH    640
#define     RESIZE_HEIGHT   480

class FrameCallback : public RaspiCallback {
    public:
        char *frame_buffer;
        int width_, height_, size_;

        // Initialize a frame buffer to contain a vcos_aligned width and height sized frame
        FrameCallback(int width, int height) : width_(VCOS_ALIGN_UP(width, 32)), height_(VCOS_ALIGN_UP(height, 16)), size_(width_ * height_) {
            frame_buffer = (char*)malloc(size_);
        }

        ~FrameCallback() {
            free(frame_buffer);
        }

        // override callback function
        void callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {
            if (buffer->length >= size_) {
                vcos_log_error("Copying grayscale data to frame buffer");
                // Copy buffer->data while the buffer is locked
                // Since the data is YUV, we are only copying the grayscale Y plane 
                memcpy(frame_buffer, buffer->data, size_);
            }
        }

        void post_process() {
            // Do any post processing to our own copy of the data after we have released the buffer
            vcos_log_error("Post processing frame buffer");
        }
};

class MotionVectorCallback : public RaspiCallback {
    public: 
        void callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer) {
            if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO) {
                vcos_log_error("Got motion vectors");
            }
        }
};

// Get the default port format's width and height
auto port_format = RaspiPort::createDefaultPortFormat();
int width = port_format.width;
int height = port_format.height;

// Create shared_ptrs for each component
auto camera = RaspiCamera::create();
auto preview_renderer = RaspiRenderer::create();
auto nullsink = RaspiNullsink::create();
auto splitter = RaspiSplitter::create();
auto encoder = RaspiEncoder::create();
auto resizer = RaspiResize::create( RESIZE_WIDTH, RESIZE_HEIGHT );

// Create shared_ptrs for callback instances
auto frameCallbackPtr = shared_ptr< FrameCallback >( new FrameCallback( RESIZE_WIDTH, RESIZE_HEIGHT ) );
auto motionVectorCallbackPtr = shared_ptr< MotionVectorCallback >( new MotionVectorCallback() );

// connect components
MMAL_STATUS_T connect_components() {
    MMAL_STATUS_T status;

    // Connect camera preview to preview renderer
    // In this case, we explicitly connect the camera's preview port since preview
    // is not the default output port
    if ((status = preview_renderer->connect(camera->preview)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't connect camera preview to preview renderer");
        return status;
    }

    if ((status = nullsink->connect(camera->still)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't connect camera still to null sink");
        return status;
    }
    // Need camera video encoding to be I420 to be able to resize or get raw output
    RASPIPORT_FORMAT_S format = camera->video->get_format();
    format.encoding = MMAL_ENCODING_I420;
    if ((status = camera->video->set_format(format)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't set camera video format to YUV");
        return status;
    }

    if ((status = splitter->connect(camera)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't connect camera video to splitter");
        return status;
    }

    if ((status = encoder->connect(splitter)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't connect encoder to resizer");
        return status;
    }

    if ((status = resizer->connect(splitter->output_1)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't resizer to splitter output_0");
        return status;
    }

    // Attach callbacks
    if ((status = resizer->output->add_callback(frameCallbackPtr)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't add frame buffer callback to resizer output");
        return status;
    }
    if ((status = encoder->output->add_callback(motionVectorCallbackPtr)) != MMAL_SUCCESS) {
        vcos_log_error("Couldn't add motion vector callback to encoder output");
        return status;
    }

    return MMAL_SUCCESS;
}

// Wait for Ctrl-C
int wait() {
    sigset_t waitset;
    int sig;
    int result = 0;

    sigemptyset( &waitset );
    sigaddset( &waitset, SIGUSR1 );
    pthread_sigmask( SIG_BLOCK, &waitset, NULL );
    result = sigwait ( &waitset, &sig );
    return 1;
}


int main(int argc, char** argv) {
    
    if (connect_components() != MMAL_SUCCESS) {
        vcos_log_error("One or more components failed to initialize");
        return -1;
    }

    vcos_log_error("Starting processing. Press Ctrl-C to exit...");
    if (camera->start() != MMAL_SUCCESS) {
        vcos_log_error("Camera failed to start");
        return -1;
    }
    vcos_log_error("Camera started");

    while (wait());

    vcos_log_error("Exiting...");
}

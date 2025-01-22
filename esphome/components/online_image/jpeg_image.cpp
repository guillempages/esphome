#include "jpeg_image.h"
#ifdef USE_ONLINE_IMAGE_JPEG_SUPPORT

#include "esphome/components/display/display_buffer.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

static const char *const TAG = "online_image.jpeg";

namespace esphome {
namespace online_image {

/**
 * @brief Callback method that will be called by the JPEGDEC engine when a chunk
 * of the image is decoded.
 *
 * @param object  The JPEGDRAW object, including the context data.
 */
static void draw_callback(const JPEGDRAW *pDraw) {
  ImageDecoder *decoder = (ImageDecoder *) pDraw->pUser;
  for (uint32_t x = 0; x < pDraw->iWidth; x++) {
    for (uint32_t y = 0; y < pDraw->iHeight; y++) {
      int pixel = x * y * 4;
      Color color(pDraw->pPixels[pixel]);
    }
  }
}

void JpegDecoder::prepare(size_t download_size) { ImageDecoder::prepare(download_size); }

int HOT JpegDecoder::decode(uint8_t *buffer, size_t size) {
  // TODO: check that download buffer is big enough
  if (size < download_size_) {
    ESP_LOGD(TAG, "Download not complete. Size: %d", size);
    return 0;
  }

  if (!this->jpeg_.openRAM(buffer, size, draw_callback)) {
    ESP_LOGE(TAG, "Could not open image for decoding.");
    return -1;
  }
  ESP_LOGD(TAG, "Image size: %d x %d, orientation: %d, bpp: %d", this->jpeg_.getWidth(), this->jpeg_.getHeight(),
           this->jpeg_.getOrientation(), this->jpeg_.getBpp());

  this->jpeg_.setUserPointer(this);
  // TODO: get image destination type and decode accordingly.
  this->jpeg_.setPixelType(RGB8888);
}

int HOT JpegDecoder::decode(uint8_t *buffer, size_t size) { return size; }

}  // namespace online_image
}  // namespace esphome

#endif  // USE_ONLINE_IMAGE_JPEG_SUPPORT

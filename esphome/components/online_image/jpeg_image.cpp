#include "jpeg_image.h"
#ifdef USE_ONLINE_IMAGE_JPEG_SUPPORT

#include "esphome/components/display/display_buffer.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "online_image.h"
static const char *const TAG = "online_image.jpeg";

namespace esphome {
namespace online_image {

/**
 * @brief Callback method that will be called by the JPEGDEC engine when a chunk
 * of the image is decoded.
 *
 * @param jpeg  The JPEGDRAW object, including the context data.
 */
static int draw_callback(JPEGDRAW *jpeg) {
  ImageDecoder *decoder = (ImageDecoder *) jpeg->pUser;

  size_t position = 0;
  for (size_t y = 0; y < jpeg->iHeight; y++) {
    for (size_t x = 0; x < jpeg->iWidth; x++) {
      auto rg = decode_value(jpeg->pPixels[position++]);
      auto ba = decode_value(jpeg->pPixels[position++]);
      Color color(rg[1], rg[0], ba[1], ba[0]);

      if (!decoder) {
        ESP_LOGE(TAG, "Decoder pointer is null!");
        return 0;
      }
      decoder->draw(jpeg->x + x, jpeg->y + y, 1, 1, color);
    }
  }
  return 1;
}

void JpegDecoder::prepare(size_t download_size) {
  ImageDecoder::prepare(download_size);
  auto size = this->image_->resize_download_buffer(download_size);
  if (size < download_size) {
    ESP_LOGE(TAG, "Resize failed!");
    // TODO: return an error code;
  }
}

int HOT JpegDecoder::decode(uint8_t *buffer, size_t size) {
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
  this->jpeg_.setPixelType(RGB8888);
  this->set_size(this->jpeg_.getWidth(), this->jpeg_.getHeight());
  ESP_LOGD(TAG, "This: %x", this);
  if (!this->jpeg_.decode(0, 0, 0)) {
    ESP_LOGE(TAG, "Error while decoding.");
    this->jpeg_.close();
    return -2;
  }
  ESP_LOGD(TAG, "JPEG Decoded!");
  this->decoded_bytes_ = size;
  this->jpeg_.close();
  return size;
}

}  // namespace online_image
}  // namespace esphome

#endif  // USE_ONLINE_IMAGE_JPEG_SUPPORT

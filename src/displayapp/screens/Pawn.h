#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"
#include "utility/DirtyValue.h"
#include "displayapp/widgets/StatusIcons.h"
#include <chrono>

#include "pawn/amx.h"
#include "pawn/amxpool.h"
#include "pawn/heatshrink_decoder.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Pawn : public Screen {
      public:
        struct File {
          virtual ~File() = default;

          virtual const uint8_t* GetConst() {
            return nullptr;
          }

          virtual void Seek(size_t position) = 0;
          virtual size_t Read(uint8_t* buffer, size_t size) = 0;

          size_t Read(uint8_t* buffer, size_t size, size_t position) {
            Seek(position);
            return Read(buffer, size);
          }
        };

        class ConstFile : public File {
          const uint8_t* backing;
          size_t size, position;

        public:
          ConstFile(const uint8_t* backing, size_t size) : backing(backing), size(size) {
          }

          const uint8_t* GetConst() override {
            return backing;
          }

          void Seek(size_t position) override {
            this->position = position;
          }

          size_t Read(uint8_t* buffer, size_t size) override {
            if (position >= this->size)
              return 0;
            if (position + size > this->size)
              size = this->size - position;
            memcpy(buffer, backing + position, size);
            position += size;
            return size;
          }
        };

        class LfsFile : public File {
          Controllers::FS& fs;
          lfs_file_t file;
          bool ok;

        public:
          LfsFile(Controllers::FS& fs, const char* path) : fs(fs) {
            ok = fs.FileOpen(&file, path, LFS_O_RDONLY) == LFS_ERR_OK;
          }

          ~LfsFile() override {
            if (ok)
              fs.FileClose(&file);
          }

          void Seek(size_t position) override {
            fs.FileSeek(&file, position);
          }

          size_t Read(uint8_t* buffer, size_t size) override {
            if (!ok)
              return 0;
            return fs.FileRead(&file, buffer, size);
          }
        };

        class HeatshrinkFile : public File {
          std::unique_ptr<File> inner;
          heatshrink_decoder decoder;
          
          size_t real_pos;
          uint8_t pending_inner_read[100];
          size_t pending_pos = 0, pending_size = 0;

          void Reset() {
            heatshrink_decoder_reset(&decoder);
            pending_size = 0;
            pending_pos = 0;
            real_pos = 0;
            inner->Seek(0);
          }

        public:
          HeatshrinkFile(std::unique_ptr<File> inner) : inner(std::move(inner)) {
            Reset();
          }

          /**
           * Seek to a specified position in the *uncompressed* file.
           */
          void Seek(size_t position) override {
            if (position < this->real_pos) // We have to rewind
              Reset();

            uint8_t discard[50];
            while (this->real_pos < position) {
              size_t remaining = position - this->real_pos;
              Read(discard, remaining > sizeof(discard) ? sizeof(discard) : remaining);
            }
          }

          size_t Read(uint8_t* buffer, size_t size) override {
            size_t actual_read, total_read = 0;

            while (total_read < size) {
              HSD_poll_res res = heatshrink_decoder_poll(&decoder, buffer + total_read, size - total_read, &actual_read);
              total_read += actual_read;
              real_pos += actual_read;

              if (res == HSDR_POLL_EMPTY) {
                if (pending_size == 0) {
                  pending_size = inner->Read(pending_inner_read, sizeof(pending_inner_read));
                  pending_pos = 0;
                }

                heatshrink_decoder_sink(&decoder, pending_inner_read + pending_pos, pending_size, &actual_read);
                pending_size -= actual_read;
                pending_pos += actual_read;
              }
            }

            return total_read;
          }
        };

        Pawn(AppControllers& controllers) : controllers(controllers) {
          // Should never be called, this constructor is only present for conformity to the apps interface
        }
        Pawn(AppControllers& controllers, std::unique_ptr<File> file);
        ~Pawn() override;

        void Refresh() override;

        void QueueError(unsigned int amx_err);
        void ShowError(unsigned int amx_err);
        void ShowError(const char* msg);

        bool OnTouchEvent(TouchEvents event) override;
        bool OnTouchEvent(uint16_t x, uint16_t y) override;

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        AppControllers& controllers;

        std::unique_ptr<Widgets::StatusIcons> statusIcons;

        amxPool amx_pool;
        std::unique_ptr<File> file;

        bool is_errored = false;

      private:
        AMX amx;

        int refresh_index, touch_index, gesture_index;
        lv_task_t* taskRefresh = 0;
        unsigned int queued_error = 0;

        std::unique_ptr<uint8_t[]> header, datablock, overlaypool, filecode;

        int LoadProgram();
        void CleanUI();
      };
    }

    template <>
    struct AppTraits<Apps::Pawn> {
      static constexpr Apps app = Apps::Pawn;
      static constexpr const char* icon = "P";

      static Screens::Screen* Create(AppControllers& /*controllers*/) {
        return nullptr;
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return false;
      };
    };
  }
}

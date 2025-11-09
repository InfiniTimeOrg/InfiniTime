#pragma once

#include "components/fs/FS.h"

using namespace Pinetime::Controllers;

namespace Pinetime {
  namespace Utility {
    template<class T>
    class Persistent {
      public:
        Persistent(Pinetime::Controllers::FS& fs);

      protected:
        Pinetime::Controllers::FS& fs;

        virtual char* GetPersistencePath() = 0;
        virtual uint32_t GetDataVersion() = 0;

        void LoadPersistentState();
        void SavePersistentState();

        struct PersistentState : T {
          uint32_t version;
        };

        PersistentState persistent_state {};
    };
    
    template<class T>
    Persistent<T>::Persistent(Pinetime::Controllers::FS& fs) : fs {fs} {
      persistent_state.version = this->GetDataVersion();
    }

    template<class T>
    void Persistent<T>::LoadPersistentState() {
      Persistent<T>::PersistentState buffer;
      lfs_file_t file;

      if (fs.FileOpen(&file, this->GetPersistencePath(), LFS_O_RDONLY) != LFS_ERR_OK) {
        return;
      }
      fs.FileRead(&file, reinterpret_cast<uint8_t*>(&buffer), sizeof(persistent_state));
      fs.FileClose(&file);
      if (buffer.version == this->GetDataVersion()) {
        persistent_state = buffer;
      }
    }

    template<class T>
    void Persistent<T>::SavePersistentState() {
      lfs_file_t file;

      if (fs.FileOpen(&file, this->GetPersistencePath(), LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
        return;
      }
      fs.FileWrite(&file, reinterpret_cast<uint8_t*>(&persistent_state), sizeof(persistent_state));
      fs.FileClose(&file);
    }
  }
}
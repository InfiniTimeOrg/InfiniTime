#include "displayapp/fonts/FastFont.h"

#include "components/fs/FS.h"

#include <type_traits>

using namespace Pinetime::Components;

namespace {
  template <typename T>
  void FixPointer(T** ptr, uintptr_t base) {
    // extremely naughty generic const removal
    using ConstFreeBase = std::remove_const_t<T>;
    using ConstFreePtr = std::add_pointer_t<std::add_pointer_t<ConstFreeBase>>;
    auto ptrStripped = const_cast<ConstFreePtr>(ptr);
    // reinterpret as a pointer to a pointer, which we can safely add to
    auto* ptrRaw = reinterpret_cast<uintptr_t*>(ptrStripped);
    *ptrRaw += base;
  }
}

lv_font_t* FastFont::LoadFont(Pinetime::Controllers::FS& filesystem, const char* fontPath) {
  int ret;
  lfs_file_t file;
  ret = filesystem.FileOpen(&file, fontPath, LFS_O_RDONLY);
  if (ret < 0) {
    return nullptr;
  }
  // Can use stat to get the size, but since the file is open we can grab it from there
  lfs_size_t size = file.ctz.size;
  void* fontData = malloc(size);
  if (fontData == nullptr) {
    filesystem.FileClose(&file);
    return nullptr;
  }
  ret = filesystem.FileRead(&file, static_cast<uint8_t*>(fontData), size);
  filesystem.FileClose(&file);
  if (ret != static_cast<int>(size)) {
    free(fontData);
    return nullptr;
  }
  auto* font = static_cast<lv_font_t*>(fontData);
  auto base = reinterpret_cast<uintptr_t>(fontData);

  // Fix LVGL fetch pointers
  font->get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt;
  font->get_glyph_bitmap = lv_font_get_bitmap_fmt_txt;

  // Fix internal pointers
  FixPointer(&font->dsc, base);
  auto* font_dsc = static_cast<lv_font_fmt_txt_dsc_t*>(font->dsc);
  FixPointer(&font_dsc->glyph_bitmap, base);
  FixPointer(&font_dsc->glyph_dsc, base);
  FixPointer(&font_dsc->cmaps, base);
  // cmaps are in RAM, so we can cast away const safely
  auto* cmaps = const_cast<lv_font_fmt_txt_cmap_t*>(font_dsc->cmaps);
  for (uint16_t i = 0; i < font_dsc->cmap_num; i++) {
    if (cmaps[i].glyph_id_ofs_list != nullptr) {
      FixPointer(&cmaps[i].glyph_id_ofs_list, base);
    }
    if (cmaps[i].unicode_list != nullptr) {
      FixPointer(&cmaps[i].unicode_list, base);
    }
  }
  if (font_dsc->kern_dsc != nullptr) {
    if (font_dsc->kern_classes == 0) {
      auto* kern_dsc = static_cast<lv_font_fmt_txt_kern_pair_t*>(const_cast<void*>(font_dsc->kern_dsc));
      FixPointer(&kern_dsc->glyph_ids, base);
      FixPointer(&kern_dsc->values, base);
    } else {
      auto* kern_dsc = static_cast<lv_font_fmt_txt_kern_classes_t*>(const_cast<void*>(font_dsc->kern_dsc));
      FixPointer(&kern_dsc->class_pair_values, base);
      FixPointer(&kern_dsc->left_class_mapping, base);
      FixPointer(&kern_dsc->right_class_mapping, base);
    }
  }

  return font;
}

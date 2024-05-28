'use strict';


const u = require('../../utils');
const build_subtables = require('../../font/cmap_build_subtables');
const Cmap = require('../../font/table_cmap');


class LvCmap extends Cmap {
  constructor(font) {
    super(font);

    this.lv_compiled = false;
    this.lv_subtables = [];
  }

  lv_format2enum(name) {
    switch (name) {
      case 'format0_tiny': return 'LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY';
      case 'format0': return 'LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL';
      case 'sparse_tiny': return 'LV_FONT_FMT_TXT_CMAP_SPARSE_TINY';
      case 'sparse': return 'LV_FONT_FMT_TXT_CMAP_SPARSE_FULL';
      default: throw new Error('Unknown subtable format');
    }
  }

  lv_compile() {
    if (this.lv_compiled) return;
    this.lv_compiled = true;

    const f = this.font;

    let subtables_plan = build_subtables(f.src.glyphs.map(g => g.code));
    let idx = 0;

    for (let [ format, codepoints ] of subtables_plan) {
      let g = this.glyphByCode(codepoints[0]);
      let start_glyph_id = f.glyph_id[g.code];
      let min_code = codepoints[0];
      let max_code = codepoints[codepoints.length - 1];

      let has_charcodes = false;
      let has_ids = false;
      let defs = '';
      let entries_count = 0;

      if (format === 'format0_tiny') {
        // use default empty values
      } else if (format === 'format0') {
        has_ids = true;
        let d = this.collect_format0_data(min_code, max_code, start_glyph_id);
        entries_count = d.length;

        defs = `
static const uint8_t glyph_id_ofs_list_${idx}[] = {
${u.long_dump(d)}
};
`.trim();

      } else if (format === 'sparse_tiny') {
        has_charcodes = true;
        let d = this.collect_sparse_data(codepoints, start_glyph_id);
        entries_count = d.codes.length;

        defs = `
static const uint16_t unicode_list_${idx}[] = {
${u.long_dump(d.codes, { hex: true })}
};
`.trim();

      } else { // assume format === 'sparse'
        has_charcodes = true;
        has_ids = true;
        let d = this.collect_sparse_data(codepoints, start_glyph_id);
        entries_count = d.codes.length;

        defs = `
static const uint16_t unicode_list_${idx}[] = {
${u.long_dump(d.codes, { hex: true })}
};
static const uint16_t glyph_id_ofs_list_${idx}[] = {
${u.long_dump(d.ids)}
};
`.trim();
      }

      const u_list = has_charcodes ? `unicode_list_${idx}` : 'NULL';
      const id_list = has_ids ? `glyph_id_ofs_list_${idx}` : 'NULL';

      /* eslint-disable max-len */
      const head = `    {
        .range_start = ${min_code}, .range_length = ${max_code - min_code + 1}, .glyph_id_start = ${start_glyph_id},
        .unicode_list = ${u_list}, .glyph_id_ofs_list = ${id_list}, .list_length = ${entries_count}, .type = ${this.lv_format2enum(format)}
    }`;

      this.lv_subtables.push({
        defs,
        head
      });

      idx++;
    }
  }

  toLVGL() {
    this.lv_compile();

    return `
/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

${this.lv_subtables.map(d => d.defs).filter(Boolean).join('\n\n')}

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
${this.lv_subtables.map(d => d.head).join(',\n')}
};
 `.trim();
  }
}


module.exports = LvCmap;

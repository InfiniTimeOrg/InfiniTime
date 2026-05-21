import os
import sys
from typing import Dict, Iterable, List, NamedTuple

from elftools.elf.elffile import ELFFile

POINTER_SIZE = 4


class Relocation(NamedTuple):
    """Reference to another section"""

    # the section referenced
    section_id: int
    # the offset within the current section of the pointer pointing to the referenced section
    offset: int


class Section(NamedTuple):
    """Corresponds to a variable in the source"""

    # the section data
    data: bytes
    # relocations within the section
    relocations: List[Relocation]


def load_data_section(section_id: int, elf_file: ELFFile) -> Section:
    """Load a section (i.e a variable) and find the sections it references (relocations)"""
    section = elf_file.get_section(section_id)
    relocations: List[Relocation] = []
    relocation_section = elf_file.get_section_by_name(".rel" + section.name)
    if relocation_section is not None:
        symbol_table = elf_file.get_section_by_name(".symtab")
        for reloc in relocation_section.iter_relocations():
            entry = symbol_table.get_symbol(reloc.entry.r_info_sym).entry
            # if not a section, ignore
            if entry.st_shndx == "SHN_UNDEF":
                continue
            assert (
                entry.st_shndx != section_id
            ), "Relocation loop, is -fdata-sections set?"
            relocations.append(Relocation(entry.st_shndx, reloc.entry.r_offset))
    return Section(section.data(), relocations)


def serialise_element(
    section_id: int, file_data: bytearray, elf_file: ELFFile
) -> Dict[int, int]:
    """Serialise a single section of the ELF (i.e one variable, as -fdata-sections used)"""
    symbols = {}
    data_section = load_data_section(section_id, elf_file)
    # align section to POINTER_SIZE
    if len(file_data) % POINTER_SIZE:
        file_data.extend(bytes(POINTER_SIZE - len(file_data) % POINTER_SIZE))
    # current length is where this section will start
    symbols[section_id] = len(file_data)
    file_data.extend(data_section.data)
    # serialise all children and store their locations
    for field in data_section.relocations:
        symbols.update(serialise_element(field.section_id, file_data, elf_file))
    # populate relative pointers to the child elements
    for field in data_section.relocations:
        file_offset = symbols[section_id]
        file_data[
            file_offset + field.offset : file_offset + field.offset + POINTER_SIZE
        ] = symbols[field.section_id].to_bytes(POINTER_SIZE, "little")
    return symbols


def main(filenames: Iterable[str]) -> None:
    """Entrypoint"""
    for filename in filenames:
        with open(filename, "rb") as elf_handle:
            elf_file = ELFFile(elf_handle)
            # get name of root font object from filename
            obj_name = os.path.split(filename)[1].removesuffix(".c.o")
            main_id = (
                elf_file.get_section_by_name(".symtab")
                .get_symbol_by_name(obj_name)[0]
                .entry.st_shndx
            )

            file_data = bytearray()
            serialise_element(main_id, file_data, elf_file)
            with open(f"{obj_name}.bin", "wb") as packed_font_handle:
                packed_font_handle.write(file_data)


if __name__ == "__main__":
    main(sys.argv[1:])

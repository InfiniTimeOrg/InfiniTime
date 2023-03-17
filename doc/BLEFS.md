# BLE FS

---

The BLE FS protocol in InfiniTime is mostly Adafruit's BLE file transfer protocol, as described in [adafruit/Adafruit_CircuitPython_BLE_File_Transfer](https://github.com/adafruit/Adafruit_CircuitPython_BLE_File_Transfer). There are some deviations, such as the status codes. These will be described later in the document.

---

## UUIDs

There are two relevant UUIDs in this protocol: the version characteristic, and the raw transfer characteristic.

### Version

UUID: `adaf0100-4669-6c65-5472-616e73666572`

The version characteristic returns the version of the protocol to which the sender adheres. It returns a single unsigned 32-bit integer. The latest version at the time of writing this is 4.

### Transfer

UUID: `adaf0200-4669-6c65-5472-616e73666572`

The transfer characteristic is responsible for all the data transfer between the client and the watch. It supports write and notify. Writing a packet on the characteristic results in a response via notify.

---

## Usage

The separator for paths is `/`, and absolute paths must start with `/`.

All of the following commands and responses are transferred via the transfer characteristic

### Read file

To begin reading a file, a header must first be sent. The header packet should be formatted like so:

- Command (single byte): `0x10`
- 1 byte of padding
- Unsigned 16-bit integer encoding the length of the file path.
- Unsigned 32-bit integer encoding the location at which to start reading the first chunk.
- Unsigned 32-bit integer encoding the amount of bytes to be read.
- File path: UTF-8 encoded string that is _not_ null terminated.

To continue reading the file after this initial packet, the following packet should be sent until all the data has been received. No close command is required after the data has been received.

- Command (single byte): `0x12`
- Status: `0x01`
- 2 bytes of padding
- Unsigned 32-bit integer encoding the location at which to start reading the next chunk.
- Unsigned 32-bit integer encoding the amount of bytes to be read. This may be different from the size in the header.

Both of these commands receive the following response:

- Command (single byte): `0x11`
- Status (signed 8-bit integer)
- 2 bytes of padding
- Unsigned 32-bit integer encoding the offset of this chunk
- Unsigned 32-bit integer encoding the total size of the file
- Unsigned 32-bit integer encoding the amount of data in the current chunk
- Contents of the current chunk

### Write file

To begin writing to a file, a header must first be sent. The header packet should be formatted like so:

- Command (single byte): `0x20`
- 1 byte of padding
- Unsigned 16-bit integer encoding the length of the file path.
- Unsigned 32-bit integer encoding the location at which to start writing to the file.
- Unsigned 64-bit integer encoding the unix timestamp with nanosecond resolution. This will be used as the modification time. At the time of writing, this is not implemented in InfiniTime, but may be in the future.
- Unsigned 32-bit integer encoding the size of the file that will be sent
- File path: UTF-8 encoded string that is _not_ null terminated.

To continue reading the file after this initial packet, the following packet should be sent until all the data has been sent and a response had been received with 0 free space. No close command is required after the data has been received.

- Command (single byte): `0x22`
- Status: `0x01`
- 2 bytes of padding.
- Unsigned 32-bit integer encoding the location at which to write the next chunk.
- Unsigned 32-bit integer encoding the amount of bytes to be written.
- Data

Both of these commands receive the following response:

- Command (single byte): `0x21`
- Status (signed 8-bit integer)
- 2 bytes of padding
- Unsigned 32-bit integer encoding the current offset in the file
- Unsigned 64-bit integer encoding the unix timestamp with nanosecond resolution. This will be used as the modification time. At the time of writing, this is not implemented in InfiniTime, but may be in the future.
- Unsigned 32-bit integer encoding the amount of data the client can send until the file is full.

### Delete file

- Command (single byte): `0x30`
- 1 byte of padding
- Unsigned 16-bit integer encoding the length of the file path.
- File path: UTF-8 encoded string that is _not_ null terminated.

The response to this packet will be as follows:

- Command (single byte): `0x31`
- Status (signed 8-bit integer)

### Make directory

- Command (single byte): `0x40`
- 1 byte of padding
- Unsigned 16-bit integer encoding the length of the file path.
- 4 bytes of padding
- Unsigned 64-bit integer encoding the unix timestamp with nanosecond resolution.
- File path: UTF-8 encoded string that is _not_ null terminated.

The response to this packet will be as follows:

- Command (single byte): `0x41`
- Status (signed 8-bit integer)
- 6 bytes of padding
- Unsigned 64-bit integer encoding the unix timestamp with nanosecond resolution.

### List directory

Paths returned by this command are relative to the path given in the request

- Command (single byte): `0x50`
- 1 byte of padding
- Unsigned 16-bit integer encoding the length of the file path.
- File path: UTF-8 encoded string that is _not_ null terminated.

The response to this packet will be as follows. Responses will be sent until the final entry, which will have entry number == total entries

- Command (single byte): `0x51`
- Status (signed 8-bit integer)
- Unsigned 16-bit integer encoding the length of the file path.
- Unsigned 32-bit integer encoding the entry number
- Unsigned 32-bit integer encoding the total amount of entries
- Flags: unsigned 32-bit integer
  - Bit 0: Set when entry is a directory
  - Bits 1-7: Reserved
- Unsigned 64-bit integer encoding the unix timestamp of the modification time with nanosecond resolution
- Unsigned 32-bit integer encoding the size of the file
- Path: UTF-8 encoded string that is _not_ null terminated.

### Move file or directory

- Command (single byte): `0x60`
- 1 byte of padding
- Unsigned 16-bit integer encoding the length of the old path
- Unsigned 16-bit integer encoding the length of the new path
- Old path: UTF-8 encoded string that is _not_ null terminated.
- 1 byte of padding
- Newpath: UTF-8 encoded string that is _not_ null terminated.

The response to this packet will be as follows:

- Command (single byte): `0x61`
- Status (signed 8-bit integer)

---

## Deviations

This section describes the differences between Adafruit's spec and InfiniTime's implementation.

### Status codes

The status codes returned by InfiniTime are a signed 8-bit integer, rather than an unsigned one as described in the spec.

InfiniTime uses LittleFS error codes rather than the ones described in the spec. Those codes can be found in [lfs.h](https://github.com/littlefs-project/littlefs/blob/master/lfs.h#L70).

import math
import pexpect
import time

from array import array
from util  import *

from nrf_ble_dfu_controller import NrfBleDfuController

verbose = False

class Procedures:
    START_DFU                   = 1
    INITIALIZE_DFU              = 2
    RECEIVE_FIRMWARE_IMAGE      = 3
    VALIDATE_FIRMWARE           = 4
    ACTIVATE_IMAGE_AND_RESET    = 5
    RESET_SYSTEM                = 6
    REPORT_RECEIVED_IMAGE_SIZE  = 7
    PRN_REQUEST                 = 8
    RESPONSE                    = 16
    PACKET_RECEIPT_NOTIFICATION = 17

    string_map = {
        START_DFU                   : "START_DFU",
        INITIALIZE_DFU              : "INITIALIZE_DFU",
        RECEIVE_FIRMWARE_IMAGE      : "RECEIVE_FIRMWARE_IMAGE",
        VALIDATE_FIRMWARE           : "VALIDATE_FIRMWARE",
        ACTIVATE_IMAGE_AND_RESET    : "ACTIVATE_IMAGE_AND_RESET",
        RESET_SYSTEM                : "RESET_SYSTEM",
        REPORT_RECEIVED_IMAGE_SIZE  : "REPORT_RECEIVED_IMAGE_SIZE",
        PRN_REQUEST                 : "PACKET_RECEIPT_NOTIFICATION_REQUEST",
        RESPONSE                    : "RESPONSE",
        PACKET_RECEIPT_NOTIFICATION : "PACKET_RECEIPT_NOTIFICATION",
    }

    @staticmethod
    def to_string(proc):
        return Procedures.string_map[proc]

    @staticmethod
    def from_string(proc_str):
        return int(proc_str, 16)

class Responses:
    SUCCESS                     = 1
    INVALID_STATE               = 2
    NOT_SUPPORTED               = 3
    DATA_SIZE_EXCEEDS_LIMITS    = 4
    CRC_ERROR                   = 5
    OPERATION_FAILED            = 6

    string_map = {
        SUCCESS                     : "SUCCESS",
        INVALID_STATE               : "INVALID_STATE",
        NOT_SUPPORTED               : "NOT_SUPPORTED",
        DATA_SIZE_EXCEEDS_LIMITS    : "DATA_SIZE_EXCEEDS_LIMITS",
        CRC_ERROR                   : "CRC_ERROR",
        OPERATION_FAILED            : "OPERATION_FAILED",
    }

    @staticmethod
    def to_string(res):
        return Responses.string_map[res]

    @staticmethod
    def from_string(res_str):
        return int(res_str, 16)


class BleDfuControllerLegacy(NrfBleDfuController):
    # Class constants
    UUID_CONTROL_POINT   = "00001531-1212-efde-1523-785feabcd123"
    UUID_PACKET          = "00001532-1212-efde-1523-785feabcd123"
    UUID_VERSION         = "00001534-1212-efde-1523-785feabcd123"

    # Constructor inherited from abstract base class

    # --------------------------------------------------------------------------
    #  Start the firmware update process
    # --------------------------------------------------------------------------
    def start(self, verbose=False):
        (_, self.ctrlpt_handle, self.ctrlpt_cccd_handle) = self._get_handles(self.UUID_CONTROL_POINT)
        (_, self.data_handle, _) = self._get_handles(self.UUID_PACKET)

        self.pkt_receipt_interval = 10

        if verbose:
            print('Control Point Handle: 0x%04x, CCCD: 0x%04x' % (self.ctrlpt_handle, self.ctrlpt_cccd_handle))
            print('Packet handle: 0x%04x' % (self.data_handle))

        # Subscribe to notifications from Control Point characteristic
        if verbose: print("Enabling notifications")
        self._enable_notifications(self.ctrlpt_cccd_handle)

        # Send 'START DFU' + Application Command
        if verbose: print("Sending START_DFU")
        self._dfu_send_command(Procedures.START_DFU, [0x04])

        # Transmit binary image size
        # Need to pad the byte array with eight zero bytes 
        # (because that's what the bootloader is expecting...)
        hex_size_array_lsb = uint32_to_bytes_le(len(self.bin_array))
        zero_pad_array_le(hex_size_array_lsb, 8)
        self._dfu_send_data(hex_size_array_lsb)

        # Wait for response to Image Size
        print("Waiting for Image Size notification")
        self._wait_and_parse_notify()

        # Send 'INIT DFU' + Init Packet Command
        self._dfu_send_command(Procedures.INITIALIZE_DFU, [0x00])

        # Transmit the Init image (DAT).
        self._dfu_send_init()

        # Send 'INIT DFU' + Init Packet Complete Command
        self._dfu_send_command(Procedures.INITIALIZE_DFU, [0x01])

        print("Waiting for INIT DFU notification")
        # Wait for INIT DFU notification (indicates flash erase completed)
        self._wait_and_parse_notify()

        # Set the Packet Receipt Notification interval
        if verbose: print("Setting pkt receipt notification interval")
        prn = uint16_to_bytes_le(self.pkt_receipt_interval)
        self._dfu_send_command(Procedures.PRN_REQUEST, prn)

        # Send 'RECEIVE FIRMWARE IMAGE' command to set DFU in firmware receive state. 
        self._dfu_send_command(Procedures.RECEIVE_FIRMWARE_IMAGE)

        # Send bin_array contents as as series of packets (burst mode).
        # Each segment is pkt_payload_size bytes long.
        # For every pkt_receipt_interval sends, wait for notification.
        segment_count = 0
        segment_total = int(math.ceil(self.image_size/float(self.pkt_payload_size)))
        time_start = time.time()
        last_send_time = time.time()
        print("Begin DFU")
        for i in range(0, self.image_size, self.pkt_payload_size):
            segment = self.bin_array[i:i + self.pkt_payload_size]
            self._dfu_send_data(segment)
            segment_count += 1

            # print "segment #{} of {}, dt = {}".format(segment_count, segment_total, time.time() - last_send_time)
            # last_send_time = time.time()

            if (segment_count == segment_total):
                print_progress(self.image_size, self.image_size, prefix = 'Progress:', suffix = 'Complete', barLength = 50)

                duration = time.time() - time_start
                print("\nUpload complete in {} minutes and {} seconds".format(int(duration / 60), int(duration % 60)))
                if verbose: print("segments sent: {}".format(segment_count))

                print("Waiting for DFU complete notification")
                # Wait for DFU complete notification
                self._wait_and_parse_notify()

            elif (segment_count % self.pkt_receipt_interval) == 0:
                (proc, res, pkts) = self._wait_and_parse_notify()

                # TODO: Check pkts == segment_count * pkt_payload_size

                if res != Responses.SUCCESS:
                    raise Exception("bad notification status: {}".format(Responses.to_string(res)))

                print_progress(pkts, self.image_size, prefix = 'Progress:', suffix = 'Complete', barLength = 50)

        # Send Validate Command
        self._dfu_send_command(Procedures.VALIDATE_FIRMWARE)

        print("Waiting for Firmware Validation notification")
        # Wait for Firmware Validation notification
        self._wait_and_parse_notify()

        # Wait a bit for copy on the peer to be finished
        time.sleep(1)

        # Send Activate and Reset Command
        print("Activate and reset")
        self._dfu_send_command(Procedures.ACTIVATE_IMAGE_AND_RESET)

    # --------------------------------------------------------------------------
    #  Check if the peripheral is running in bootloader (DFU) or application mode
    #  Returns True if the peripheral is in DFU mode
    # --------------------------------------------------------------------------
    def check_DFU_mode(self):
        if verbose: print("Checking DFU State...")

        cmd = 'char-read-uuid %s' % self.UUID_VERSION

        if verbose: print(cmd)

        self.ble_conn.sendline(cmd)

        # Skip two rows
        try:
            res = self.ble_conn.expect('handle:.*', timeout=10)
            # res = self.ble_conn.expect('handle:', timeout=10)
        except pexpect.TIMEOUT as e:
            print("State timeout")
        except:
            pass

        return self.ble_conn.after.find(b'value: 08 00')!=-1

    def switch_to_dfu_mode(self):
        (_, bl_value_handle, bl_cccd_handle) = self._get_handles(self.UUID_CONTROL_POINT)

        # Enable notifications
        cmd = 'char-write-req 0x%02x %02x' % (bl_cccd_handle, 1)
        if verbose: print(cmd)
        self.ble_conn.sendline(cmd)

        # Reset the board in DFU mode. After reset the board will be disconnected
        cmd = 'char-write-req 0x%02x 0104' % (bl_value_handle)
        if verbose: print(cmd)
        self.ble_conn.sendline(cmd)

        time.sleep(0.5)

        #print  "Send 'START DFU' + Application Command"
        #self._dfu_state_set(0x0104)

        # Reconnect the board.
        #ret = self.scan_and_connect()
        #if verbose: print("Connected " + str(ret))

        #return ret
        return 1


    # --------------------------------------------------------------------------
    #  Parse notification status results
    # --------------------------------------------------------------------------
    def _dfu_parse_notify(self, notify):
        if len(notify) < 3:
            print("notify data length error")
            return None

        if verbose: print(notify)

        dfu_notify_opcode = Procedures.from_string(notify[0])

        if dfu_notify_opcode == Procedures.RESPONSE:

            dfu_procedure = Procedures.from_string(notify[1])
            dfu_response  = Responses.from_string(notify[2])

            procedure_str = Procedures.to_string(dfu_procedure)
            response_str  = Responses.to_string(dfu_response)

            if verbose: print("opcode: 0x%02x, proc: %s, res: %s" % (dfu_notify_opcode, procedure_str, response_str))

            return (dfu_procedure, dfu_response)

        if dfu_notify_opcode == Procedures.PACKET_RECEIPT_NOTIFICATION:
            receipt = bytes_to_uint32_le(notify[1:5])
            return (dfu_notify_opcode, Responses.SUCCESS, receipt)

    # --------------------------------------------------------------------------
    #  Wait for a notification and parse the response
    # --------------------------------------------------------------------------
    def _wait_and_parse_notify(self):
        if verbose: print("Waiting for notification")
        notify = self._dfu_wait_for_notify()

        if notify is None:
            raise Exception("No notification received")

        if verbose: print("Parsing notification")

        result = self._dfu_parse_notify(notify)
        if result[1] != Responses.SUCCESS:
            raise Exception("Error in {} procedure, reason: {}".format(
                Procedures.to_string(result[0]),
                Responses.to_string(result[1])))

        return result

    #--------------------------------------------------------------------------
    # Send the Init info (*.dat file contents) to peripheral device.
    #--------------------------------------------------------------------------
    def _dfu_send_init(self):
        if verbose: print("dfu_send_init")

        # Open the DAT file and create array of its contents
        init_bin_array = array('B', open(self.datfile_path, 'rb').read())

        # Transmit Init info
        self._dfu_send_data(init_bin_array)

BLE Stress Tests

******************************************************************************
    QUICK TIPS:
    You need 2 controllers supported by MyNewt. One will become TX device,
    the other will become RX device.

    1. Set role (TX=0, RX=1) for current device in syscfg.yml
        BLE_STRESS_TEST_ROLE: 1

       The RX has LED2 turned on.

    2. Set (in syscfg.yml) number of times to repeat each tested action
       in use case, e.g. do 1000 times connect/disconnect to complete use case.
        RX_STRESS_REPEAT: 1000

    3. To perform only specific test, go to rx_stress.c,
       find definition of rx_stress_main_task_fn(void *arg) and in place of
       for-loop just paste function rx_stress_start(i), where i is id of test.

******************************************************************************

No | Use case
-----------------------------------------------------------------------------
 1 | Stress Connect -> Connect Cancel - repeat 1000
   | RX: Nothing
   | TX: Connect/Connect cancel
   |
 2 | Stress Connect/Disconnect legacy - repeat 1000
   | RX: Advertise legacy
   | TX: Connect/Disconnect
   |
 3 | Stress Connect/Disconnect ext adv - repeat 1000
   | RX: Advertise Ext
   | TX: Connect/Disconnect
   |
 4 | Stress connection params update (TX) - (1000)
   | RX: Advertise
   | TX: Connect/Connect param update
   |
 5 | Stress connection params update (RX) - (1000)
   | RX: Advertise/Connect param update
   | TX: Connect
   |
 6 | Stress Scan
   | RX: Advertise a known data pattern
   | TX: Scan and check received data with pattern
   |
 7 | Stress PHY Update (TX) - (1000)
   | RX: Advertise
   | TX: Connect/Phy update
   |
 8 | Stress PHY update (RX) - (1000)
   | RX: Advertise/Phy update
   | TX: Connect
   |
 9 | Stress multi connection
   | RX: Advertise Ext
   | TX: Establish and maintain as many instances as possible
   |
10 | Stress L2CAP send
   | RX: Send 64kB of data with L2CAP
   | TX: Measure bit rate and max received data MTU
   |
11 | Stress Advertise/Connect/Continue Adv/Disconnect
   | RX: Advertise Ext/Continue same advertise after connect
   | TX: Connect
   |
12 | Stress GATT indicating
   | RX: Indicate
   | TX: Receive indication. Measure average time of indicating.
   |
13 | Stress GATT notification
   | RX: Notify. Measure average time of notifying.
   | TX: Count the number of received notification.
   |
14 | Stress GATT Subscribe/Notify/Unsubscribe
   | RX: Notify on subscribe
   | TX: Measure the average time from sending a subscription request
   |     to receiving a notification.
   |
15 | Stress Connect/Send/Disconnect
   | RX: Advertise/Send via ATT/Disconnect
   | TX: Receive notification. Measure time of notifying.

******************************************************************************
   Concept:
   The RX device advertises data containing a UUID128 of test use case that
   should be performed. The TX device scan for known UUIDs, when it finds,
   adapts to the advertised use case and runs a test.

   Stress Task vs Semaphore:
   The rx_stress_start_auto function starts main stress test task that runs
   all stress tests one by one. The tests are based on event handling, so to
   synchronize main task with events, a semaphore is used. On use case start
   there is 0 tokens for semaphore. After main task starts one of use cases,
   it comes across a semaphore and has to wait. When use case is completed,
   1 token is released, so main task can use it to pass through semaphore.
   The tx_stress_start_auto function works analogically.


   Newt target set example:
    rymek@rymek:~/projects/bletiny_proj$ newt target show bletest_tx
    targets/bletest_tx
        app=@apache-mynewt-nimble/apps/blestress
        bsp=@apache-mynewt-core/hw/bsp/nordic_pca10056
        build_profile=debug
        syscfg=BLE_STRESS_TEST_ROLE=0
    rymek@rymek:~/projects/bletiny_proj$ newt target show bletest_rx
     targets/bletest_rx
        app=@apache-mynewt-nimble/apps/blestress
        bsp=@apache-mynewt-core/hw/bsp/nordic_pca10056
        build_profile=debug
        syscfg=BLE_STRESS_TEST_ROLE=1


   Example of expected logs on TX side(LOG_LEVEL > 1):
     Start test num 1
     >>>>>>>>>>>>>>>>>>>> Stress test 1 completed
     Start scan for test
     Start test num 2
     >>>>>>>>>>>>>>>>>>>> Stress test 2 completed
     Start scan for test
     Start test num 3
     >>>>>>>>>>>>>>>>>>>> Stress test 3 completed
     Start scan for test
     Start test num 4
     >>>>>>>>>>>>>>>>>>>> Stress test 4 completed
     Start scan for test
     Start test num 5
     >>>>>>>>>>>>>>>>>>>> Stress test 5 completed
     Start scan for test
     Start test num 6
     >>>>>>>>>>>>>>>>>>>> Stress test 6 completed
     Start scan for test
     Start test num 7
     >>>>>>>>>>>>>>>>>>>> Stress test 7 completed
     Start scan for test
     Start test num 8
     >>>>>>>>>>>>>>>>>>>> Stress test 8 completed
     All tests completed
     Tests results:
     Use case 1 - Stress Connect -> Connect Cancel:
     Con attempts = 20
     Con success = 0
     Use case 2 - Stress Connect/Disconnect legacy:
     Con attempts = 20
     Con success = 20
     Use case 3 - Stress Connect/Disconnect ext adv:
     Con attempts = 20
     Con success = 20
     Use case 4 - Stress connection params update (TX):
     Params updates = 20
     Use case 5 - Stress connection params update (RX):
     Params updates = 20
     Use case 6 - Stress Scan:
     Received first packets = 20
     Received all packets = 20
     Use case 7 - Stress PHY Update (TX):
     PHY updates = 20
     Use case 8 - Stress Connect -> Connect Cancel:
     PHY updates = 20


   Example of expected logs on RX side(LOG_LEVEL > 1):
     Start test num 2
     >>>>>>>>>>>>>>>>>>>> Stress test 2 completed
     Start test num 3
     >>>>>>>>>>>>>>>>>>>> Stress test 3 completed
     Start test num 4
     >>>>>>>>>>>>>>>>>>>> Stress test 4 completed
     Start test num 5
     >>>>>>>>>>>>>>>>>>>> Stress test 5 completed
     Start test num 6
     Received signal to switch test
     Start test num 7
     >>>>>>>>>>>>>>>>>>>> Stress test 7 completed
     Start test num 8
     >>>>>>>>>>>>>>>>>>>> Stress test 8 completed
     All tests completed
     Tests results:
     Use case 1 - Stress Connect -> Connect Cancel:
     Con attempts = 0
     Con success = 0
     Use case 2 - Stress Connect/Disconnect legacy:
     Con attempts = 20
     Con success = 20
     Use case 3 - Stress Connect/Disconnect ext adv:
     Con attempts = 20
     Con success = 20
     Use case 4 - Stress connection params update (TX):
     Params updates = 20
     Use case 5 - Stress connection params update (RX):
     Params updates = 20
     Use case 6 - Stress Scan:
     Received first packets = 0
     Received all packets = 0
     Use case 7 - Stress PHY Update (TX):
     PHY updates = 20
     Use case 8 - Stress Connect -> Connect Cancel:
     PHY updates = 20

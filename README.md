# AkwardClock - A special gift for my mother in law

## Lord Vetinari Clock
"*Someone very clever must have made the clock for the Patrician's waiting room. It went tick-tock like any other clock. But somehow, and against all usual horological practice the tick and the tock were irregular. Tick tock tick... and then the merest fraction of a second longer before ... tock tick tock... and then a tick a fraction of a second earlier than the mind's ear was now prepared for. The effect was enough, after ten minutes, to reduce the thinking processes of even the best-prepared to a sort of porridge. The Patrician must have paid the clockmaker quite highly.*"

![](https://user-images.githubusercontent.com/1030319/113523900-44bb3d80-95ab-11eb-8131-4f2b50d79640.gif)

## Why
I have wanted to make a "Lord Vetinari Clock" for so long. I finally got an idea on how to implement it after reading into PRNGs in [Serious Cryptography](https://nostarch.com/seriouscrypto). This implementation uses the [Xorshift32](https://en.wikipedia.org/wiki/Xorshift) which is a subset of LFSRs within the PRNG group.

One special feature of using the Xorshift32 is that the randomness period of the clock should be around 136 years. But no guarantees can be made given the way it is used. But I bet a beer that you would need to replace the batteries way before any repeat happens.

The program is actually quite simple:
1. Initialize hardware
2. Insert 64 one bits into an array of 256 bits
3. Permutate the array 256 times by exchanging two random bits
4. Enable a 250 ms interrupt timer
5. Go to sleep.

On every timer interrupt do the following:
1. Pulse the clock module if the next array bit is 1
2. Advance bit array pointer
3. Permutate the bit array every 64 s and reset the pointer

## Hardware
Much care has been taken even so the hardware is very simple and I have timed the functions to ensure that no tick is lost. I initially wanted to make a small module to be concealed within the original clock module but the prototype is already fully working and other projects has captured my attention.

### Clocking
 My first approach was to clock both the timer and the MCU using the external 32.768 kHz crystal. But I ended up using the faster internal RC clock for the MCU since I couldn't guarantee that the permutation function would complete within a 220 ms timespan (250-30). The 32.768 kHz clock is still used for precise time keeping purposes.

### Pulsing the clock module
A very slow AC square wave is used to advance the hands attached to the clock module. Normally the module is pulsed every 1 s (1/2 wave) but some newer and "silent" clocks get pulsed every 250 ms in order to make smaller advances and hence less audible ticks. They will not work with this project. Only use clock modules that advances the second hand in 1 s steps.

### 3 volt vs 1.5 volt
The clock module used here is normally supplied with 1.5 v but connecting the coil to two I/O pins and supplying the MCU with 3 v allows both for the MCU to run within spec but also makes the clock modules tick sound to be more noticeable.

### Can we go back in time ?
Sure we can, try playing with the pulse length. I have noticed that a pulse length of 5ms makes the clock go backwards. So a future project could be to incorporate this as yet another element.

## Video
[Video of the clock in action](https://youtu.be/nkP1kM_7p7E)

## Images
![Final clock](https://user-images.githubusercontent.com/1030319/113522806-f5bdda00-95a3-11eb-9f69-e1c190fcd461.jpeg)

![Backside](https://user-images.githubusercontent.com/1030319/113522820-00786f00-95a4-11eb-8c71-243c24cf8fe3.png)

![Coil wires](https://user-images.githubusercontent.com/1030319/113522831-0e2df480-95a4-11eb-8539-7a74c3b2884c.jpeg)

![Bare module](https://user-images.githubusercontent.com/1030319/113522835-15ed9900-95a4-11eb-8b57-4ee8e498a561.jpeg)

![Cogs](https://user-images.githubusercontent.com/1030319/113522850-2bfb5980-95a4-11eb-8e79-fd868b8c26f1.jpeg)

## Prior work
[https://www.instructables.com/Lord-Vetinari-Clock/](https://www.instructables.com/Lord-Vetinari-Clock/)

[https://roryokane.github.io/vetinari-clock-simulator/](https://roryokane.github.io/vetinari-clock-simulator/)

[https://hackaday.com/tag/vetinari-clock/](https://hackaday.com/tag/vetinari-clock/)

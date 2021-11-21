# BRAW Decode

This is a project that uses the official Blackmagic Raw SDK to decode a 
`*.braw` file in a way that can be read by FFmpeg. The goal of the 
project is to allow unattended, headless, conversion of `*.braw` files
into other file formats.

## Examples

I'll start right off by showing you some examples of how the 
`braw-decode` program may be used.

Generate proxy editing files:

```
braw-decode -s 2 sample.braw | ffmpeg -y -i sample.braw  $(braw-decode -s 2 -f sample.braw) \
-map 1:v:0 -map 0:a:0 -c:a copy \
-c:v dnxhd -vf "scale=1280:720,format=yuv422p" -b:v 60M  output.mov
```

Convert to 10b 32Mbps h265 for archiving using NVENC using 16 bit source:

```
braw-decode -v -c 16pl sample.braw | ffmpeg -y -i sample.braw $(braw-decode -c 16pl -f sample.braw) \
-map 1:v:0 -map 0:a:0 -c:a copy \
-c:v hevc_nvenc -b:v 32M -profile:v rext output.mov
```

## Usage

`braw-decode` has two main purposes in the conversion process.

 1. Decode the `*.braw` file into a format FFMpeg can read
 2. Provide FFmpeg with the parameters to interpret the decoded data
 
The first stage of `braw-decode` will read a `*.braw` as pure RGB 
data out to the standard CLI output. This is intended to be piped 
directly to FFmpeg. Running `braw-decode` without piping it to FFmpeg
will have unintended consequences as it will print all command 
characters that happen to be in the `*.braw` file without escaping 
them.

The second stage prints the parameters needed for FFmpeg to read them
decoded data. The program controls the following parameters:

 - `-f rawvideo` : Tells `ffmpeg` that it will be decoding a raw data stream
 - `-pixel_format` : The layout and bit depth of the data stream
 - `-s` : The intended resolution of the incoming data stream
 - `-r` : The intended frame rate of the incoming data stream
 - `-i pipe:0` : Tells `ffmpeg` to read the data from `stdin`
 - Optional `-filter:v colorchannelmixer=0:1:0:0:0:0:1:0:1:0:0:0` : Needed to correct color channels\*
 

 *\*FFmpeg does not have a planar 16 bit format in RGB channel sequence. The `colorchannelmixer` is used to correct the colors as a filter stage*

You may choose not to use `braw-decode` to control the input
parameters but you will need to specify them manually in its place.
 
`braw-decode` does nothing with audio at all as it is not needed.
`*.braw` files use the `mov` container format and a standard 
`pcm_s24le` audio stream. This can be read directly by FFmpeg
without conversion by using a `*.braw` file as an input and mapping
its audio channel to the output. The examples demonstrate this.
 
 
## Options
 
Some options of the Blackmagic Raw SDK that control the decoding of
`*.braw` files have been exposed as arguments for `braw-decode`. The
following complete listing of the arguments may be printed at any
time by running `braw-decode -h`:

```
braw-decode - BRAW file decoder
Usage: braw-decode sample.braw | ffmpeg -y $(braw-decode -f sample.braw) -c:v hevc_nvenc output.mov
 -n, --info
        Print details of clip
 -v, --verbose
        Print more information to CERR while processing
 -f, --ff-format
        Print FFmpeg arguments for processing decoded video
        Example: '-f rawvideo -pixel_format rgba -s 3840x2160 -r 60 -i pipe:0`
 -h, --help
        Print help text
 -c, --color-format [VALUE]
        Bit depth and order of color information to be output:
        Options:
                rgba - Unsigned 8bit interleaved RGBA (FFmpeg format: rgba)
                bgra - Unsigned 8bit interleaved BGRA (FFmpeg format: bgra)
                16il - Unsigned 16bit interleaved RGB (FFmpeg format: rgb48le)
                16pl - Unsigned 16bit planar RGB      (FFmpeg format: gbrp16le)
                f32s - Floating point interleaved RGB
                f32p - Floating point planar RGB
                f32a - Floating point interleaved BGRA
 -t, --threads [VALUE]
        Number of CPU threads to use for decoding
 -i, --in [VALUE]
        Start frame index for decoding
 -o, --out [VALUE]
        End frame index for decoding
 -s, --scale [VALUE]
        Scale input video down by this factor.
        WARNING: Only the 8bit color formats at half scale seem to work
        Options:
                1
                2
                4
                8
```
 
Most of these are well enough documented there but some could use 
additional information:
 
#### Verbose

Verbose mode in `braw-decode` must print messages to `stderr` as
`stdout` is used to send data to FFmpeg. Additionally it will print
real time information about the current frame and active number
of CPU threads on a single line intended to provide more information
alongside the normal FFmpeg encoding status.

#### Color Format

All supported methods of decoding `*.braw` files supported by the 
SDK have been made available, but not all are compatible with 
FFmpeg. `rawvideo` sources must be explicitly decoded in specific
ways. While FFmpeg has support for 32 bit floats, in my testing
none of the available formats match those used in the SDK. As such
they should be considered non-functional when using `braw-decode`
with FFmpeg.

Additionally, as mentioned in **Usage**, the 16 bit planar format
that FFmpeg supports is BGR formatted while the SDK outputs in RGB.
This can be corrected after encoding or as a filter stage though 
so it has been marked as compatible. I felt this to be important
as the 16 bit planar format is faster than the interleaved format 
based on my testing.

#### Scale

The scale option is handled by the SDK which is important because
it means less data is sent over `stdout` improving performance. But
in my testing the scale feature has issues with all color formats
except the 8 bit ones and only 2 or half scaling works with that. As
such I consider the scaling option really only usable for generating
proxy files due to the lower quality, but this still has real world
use so it has been left in.

The remaining scale factors have been included in the program in the
hopes that an updated SDK revision will correct the issues.

## Decoding Compute Source and Platforms Supported

The Blackmagic Raw SDK supports CPU, OpenCL, CUDA, and Metal 
decoding. As of right now `braw-decode` only supports CPU decoding.
The SDK does not come with examples that demonstrate OpenCL or Metal
configurations and I have been told by Blackmagic that the CUDA 
sample is [not configured correctly](https://forum.blackmagicdesign.com/viewtopic.php?f=12&t=150108#p801585).
Once an sample of how to configure the hardware accelerated 
decoding methods is available it will be added.

Currently `braw-decode` has only been compiled for and tested with
Linux. It should be possible to build versions for Windows and MacOS
as the SDK is available for both platforms. I will be starting
development and testing on a Windows version when time is available.
I cannot produce and test a MacOS version as I do not own a 
sufficiently new enough Apple computer to work with.

## Compiling and Setting Up

The Blackmagic Raw SDK is the only dependency needed to compile
`braw-decode`. It is available from Blackmagic here: [https://www.blackmagicdesign.com/products/blackmagicraw](https://www.blackmagicdesign.com/products/blackmagicraw)

`braw-decode` was developed using Version 2.2 of the SDK and the it
has changed multiple times so the following may need to be updated
for later versions.

Setting up the SDK

 1. Extract the downloaded Blackmagic_RAW_Linux_*.*.tar.gz | `tar -xzf Blackmagic_RAW_Linux*.tar.gz`
 2. Open the Blackmagic RAW folder and extract the rpm | `cd Blackmagic\ RAW && rpm2cpio *.rpm | cpio -idmv`
 3. Go to usr/lib64/blackmagic/BlackmagicRAWSDK/Linux/ | `cd usr/lib64/blackmagic/BlackmagicRAWSDK/Linux/`
 4. Copy the contents of the Include and Libraries folders to the one for `braw-decode`
 
You are now ready to build `braw-decode`. The only step needed is:
```
make
```

You should now have a `braw-decode` executable to run!

A note though, by default `braw-decode` looks for the "Libraries"
folder in the path that it is called from. So if you don't run it
from the folder with "Libraries" in it it will crash. If you want
make the program for from anywhere you need to copy contents of the 
Libraries folder somewhere like `/usr/lib64/brawsdk/` or 
`/opt/brawsdk/` and change the path in `braw.h` that is assigned to
`lib` on line 113 and recompile. With a full path it can find the 
files from anywhere.

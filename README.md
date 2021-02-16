# ryzen_monitor
Monitor power information of Ryzen processors via the PM table of the SMU.

This tool is based on the [ryzen_smu](https://gitlab.com/leogx9r/ryzen_smu) kernel module for reading the PM Table from the SMU. It is a continuation of the demo-tool provided with that project.

**ryzen_monitor** features support for multiple PM table versions (i.e. multiple bios versions), adds support for Ryzen 5000, and presents more fields to user. It is especially focused around providing a more realistic image of the actual power draw and hence true thermal output of the CPU package.

## Supported CPUs
* Ryzen 5000 series
* Ryzen 3000 series

## Screenshot
![ryzen monitor](https://user-images.githubusercontent.com/50338637/107124627-4e733e00-68a5-11eb-9596-bb3d35120a73.png)

## Building
First install the kernel module from [ryzen_smu](https://gitlab.com/leogx9r/ryzen_smu).

Then pull and make **ryzen_monitor**:
```bash
git clone https://github.com/hattedsquirrel/ryzen_monitor.git
cd ryzen_monitor/
make

sudo ./src/ryzen_monitor
```
Enjoy!

## About the quality of the provided information
Don't rely on the information given by this tool.

To my knowledge there is no official documentation on how to do this. Everything was created by starring at numbers, a lot of guesswork and finding fragments somewhere on the web. It is highly likely that some assignments or calculations will be incorrect.

This program is provided as is. If anything, it is a toy for the curious. Nothing more. Use it at your own risk.
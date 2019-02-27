# What is this

A toy example to segment bad apple and replay it on terminal.

    youtube-dl https://www.youtube.com/watch?v=FtutLA63Cp8 --output test
    mkdir frames
    ffmpeg -i test.mkv -vf fps=30/1 frames/frame%08d.ppm

    make
    ./main --subsample 10 --frames ./frames --fps 30

# Is it possible to use different video with different resolution?

Shouldn't be hard to modify.

# Dependency:

You need ffmpeg to segment the video into ppm:

    sudo apt install ffmpeg

You need gflags to parse the command line:
    
    sudo apt install libgflags-dev


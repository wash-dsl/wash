#!/bin/python3

import imageio
# from pygifsicle import optimize

def create_gif_from_png(path_name, frame_count, output):
    filenames = [ path_name + "_" + "{:04d}".format(x) + ".png" for x in range(frame_count) ]
    with imageio.get_writer(output + ".gif", mode="I", duration=(1.0/60.0)) as writer:
        for (k, filename) in enumerate(filenames):
            image = imageio.imread(filename)
            writer.append_data(image)
            print(k, "/", frame_count, end="      \r")
    # print("\nOptimizing...", end="\r")
    # optimize(output + ".gif")
    print("Done!         ")

if __name__ == "__main__":
    path_name = input("Input Path (no ext): ")
    frame_count = int(input("Frame count: "))
    output = input("Output Path (no ext): ")
    create_gif_from_png(path_name, frame_count, output)
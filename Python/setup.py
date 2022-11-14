from setuptools import setup
from distutils.core import Extension

def main():
    setup(
        name="lorapy",
        version="1.0",
        description="LoRa Dragino Driver",
        author="Benjamin Wirtz",
        author_email="admin@bedevin.com",
        url="https://www.bedevin.com",
        ext_modules=[
            Extension(
                "lorapy",
                ["LoRa.c"],
                extra_link_args=["-ldl", "-lwiringPi", "-lLoRaDraginoDriver"],
            )
        ]
    )


if __name__ == "__main__":
    main()

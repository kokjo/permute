#!/bin/sh
docker build -t permute .
docker run permute cat /code/program > program
chmod +x program

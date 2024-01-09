#!/bin/bash
ps -e | grep peer | cut -c1-6 | xargs kill -9 &

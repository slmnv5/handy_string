#!/bin/bash

git checkout --orphan temp

git add -A
git commit -m. 
git branch -D main
git branch -m main
git push -f origin main
git push --set-upstream origin main



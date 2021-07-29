#!/bin/bash

if [ $(git remote -v | wc -l) -ne 4 ]
then
  echo "Unusual number of remotes"
  git remote -v
fi

if [ $(git remote -v | grep "https://github.com/google/orbit.git" | wc -l) -ne 2 ]
then
  echo "Unexpected upstream remotes"
  git remote -v
fi

if [ $(git remote -v | grep -v "https://github.com/google/orbit.git" | wc -l) -ne 2 ]
then
  echo "Unexpected personal forks"
  git remote -v
fi

UPSTREAM=$(git remote -v | grep "https://github.com/google/orbit.git (fetch)" | sed -e "s/\s.*//");
echo "The upstream remote you use to fetch from github is called: \"$UPSTREAM\""
PERSONAL=$( git remote -v | grep -v "https://github.com/google/orbit.git"| grep "(push)" | sed -e "s/\s.*//")
echo "The personal fork you push to is called: \"$PERSONAL\""



#! /bin/sh
result = $(ls ./forms)
if [ "$?" = 0 ]
then
  mv ./forms/* ./src
  rm -r ./forms
fi

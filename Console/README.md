# COMET Servants, May 2025

## Background
The Servants project builds on the C# Servants created in 2020.  This project built on the AWS Cloud computing (by Joshua King) and this was built on the Galaxy Software (C++) created by John Revil.  So this is the 4th version.

## Scope

The major scope changes in this version include keeping all files locally, not using the COMET Hub, rather build one on this machine.

The idea is to have both local processing and remote processing (Hub) in this software.

## Distributed processing

While one machine must act as a "Hub" to cordinate the processing, several machines may act as processors of COMET Cases or "jobs".  One machine could also do both tasks.

## Servant HUB

The hub will get information from machines when a case is ready for processing and it will decide where it should be run.

The Hub will be contacted by other Servants to let it know how many jobs can be processed (related to available cores).


## Setting up a New Development  Machine
Mac
```bash
brew install doxygen
brew install graphviz
```

# AnaTools

## Installation instructions

```
cmsrel CMSSW_10_6_16
cd CMSSW_10_6_16/src
cmsenv

#This package
cd $CMSSW_BASE/src
git clone https://github.com/michael-pitt/AnaTools.git
scram b -j 8
```

## Table of content

The repository contains several tools to analyze samples with [CMSSW](https://cms-sw.github.io/)

### 1. LHEAnalysis
The package is used to assess LHE information and drop it into a ROOT file.
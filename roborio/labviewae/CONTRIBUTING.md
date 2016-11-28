# Contributing to the navX LabVIEW Library
The FIRST Robotics Community is a great resource for help and it is what makes the games enjoyable year after year.

## Install GitHub Desktop for Windows
[GitHub Desktop](https://desktop.github.com/)

### Git(Hub) and LabVIEW
Making LabVIEW work with Git is not as simple as it is with text based languages.
Pulled from [Revision Control with Git for FRC Teams](https://docs.google.com/document/pub?id=1Lmx9WI1g_ObB03Vrlfb7QU3ochz1q_YRGM8dPi0R8g8).
#### Enabling Merge and Diff for LabVIEW
```
git config --system merge.labview.name "LabView 3-Way Merge"
git config --system merge.labview.driver '"C:\Program Files\National Instruments\Shared\LabVIEW Merge\LVMerge.exe" "C:\Program Files\National Instruments\LabVIEW 8.6\LabVIEW.exe" %O %B %A %A'
git config --system merge.labview.recursive binary
git config --system diff.labview.command '"C:\Program Files\National Instruments\LabVIEW 8.6\LabVIEW.exe" "C:\path\to\diff.vi" --'
```
__If using Powershell__
```
git config --system merge.labview.name "LabView 3-Way Merge"
git config --system merge.labview.driver "\`"C:\Program Files\National Instruments\Shared\LabVIEW Merge\LVMerge.exe\`" \`"C:\Program Files\National Instruments\LabVIEW 8.6\LabVIEW.exe\`" %O %B %A %A"
git config --system merge.labview.recursive binary
git config --system diff.labview.command "\`"C:\Program Files\National Instruments\LabVIEW 8.6\LabVIEW.exe\`" \`"C:\path\to\diff.vi\`" --"
```

#### .gitattributes
```
# Use a custom driver to diff merge LabVIEW files
*.vi merge=labview diff=labview
```

## Getting Started
* Make sure you have a [GitHub account](https://github.com/signup/free)
* [Submit an issue](https://github.com/FRC900/navX-MXP-LabVIEW/issues/new), assuming one does not already exist.
  * Clearly describe the issue including steps to reproduce when it is a bug.
  * Make sure you fill in the earliest version that you know has the issue.
* Fork the repository on GitHub

### Making Changes
* Create a topic branch from where you want to base your work.
  * This is usually the master branch.
  * Only target release branches if you are certain your fix must be on that
    branch.
  * To quickly create a topic branch based on master; `git checkout -b
    fix/master/my_contribution master`. Please avoid working directly on the
    `master` branch.
* Make commits of logical units.
* Make sure your commit messages are sensical.
# CC1muAnalysis

Events simulated with different generators (GENIE, GiBUU, etc.) are analyzed and different single and double differential plots are created. 

### Setup

Assuming the [`BuildEventGenerators`](https://github.com/afropapp13/BuildEventGenerators) repository was used to generate the events, the only setup needed is

```bash
/exp/sbnd/app/users/${user}/BuildEventGenerators/setup_generators.sh
```

(or pointed to wherever the `BuildEventGenerators` repository was cloned).

### Running scripts

The file that processes all the `.flat.root` files is ran by 

```bash
root -b Scripts/script_LoopGenerators.C 
```

This will create `.root` files with the analyzed data ready to create plots. The plots are generated by calling 

```bash
root -l Scripts/${filename}
```

where `${filename}` can be one of the following:

- `GeneratorOverlay.cpp`: creates overlaid plots for all the available variables and generators.
- `GeneratorInteBreakDown.cxx`: creates a plot with event interaction type breakdown for each plot and generator.
- `SerialGeneratorOverlay.cpp`: creates sliced plots for the double differential variables and normal generators.
- `MECGeneratorOverlay.cpp`: same as `GeneratorOverlay.cpp` but for purely MEC generators.
- `MECSerialGeneratorOverlay.cpp`: same as `SerialGeneratorOverlay.cpp` but for purely MEC generators.
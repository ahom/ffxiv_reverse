ffxiv_reverse
===========

*This repository is not maintained anymore, however it could well still work*

You can issue pull requests, and I will make sure to integrate them if they make sense, but I think you're better off forking this repository. I hope you'll find the content interesting enough!

*Note*: These libs has the following dependencies:
  - zlib
  - boost
  - cmake

## Interesting things to see

* How to read the data files
  - [xiv/dat](https://github.com/ahom/ffxiv_reverse/tree/master/xiv/dat)

* How to read the csv files (the actual database)
  - [xiv/exd](https://github.com/ahom/ffxiv_reverse/tree/master/xiv/exd)
  
* How to read the texture files
  - [xivp/tex](https://github.com/ahom/ffxiv_reverse/tree/master/xivp/tex)
  
* How to read the model files
  - [xivp/mdl](https://github.com/ahom/ffxiv_reverse/tree/master/xivp/mdl)
  - This is what has been used for (http://xivmodels.com/)[http://xivmodels.com/]
  - Note that there are links between material/shader files (in addition to variable matrices) and one could use that to display models more accurately, but as xivmodels is powered by WebGL, the DirectX ones where not that useful to me.

## Author

* [Antoine Hom](https://github.com/ahom)

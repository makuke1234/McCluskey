# McCluskey lahendaja

Lahendab McCluskey intervallmeetodiga nii MDNK-sid (minimaalseid disjunktiivseid
normaalkujusid) kui ka MKNK-sid (minimaalseid konjunktiivseid normaalkujusid).

*Solves minimal disjunctive normal forms and minimal conjunctival normal forms*
*using McCluskey's interval method.*

[![Release version](https://img.shields.io/github/v/release/makuke1234/McCluskey?display_name=release&include_prereleases)](https://github.com/makuke1234/McCluskey/releases/latest)
[![Total downloads](https://img.shields.io/github/downloads/makuke1234/McCluskey/total)](https://github.com/makuke1234/McCluskey/releases)
![C version](https://img.shields.io/badge/version-C2x-blue.svg)


# Kasutamine

Sisendisse tuleb anda igale reale vastav argumentvektor ning sellele vastav väärtus
tõeväärtustabelist.
* Ühtede režiimis on toetatud kujud:
	* [argumentvektor] 1
	* [argumentvektor] -
* Nullide režiimis (tuleb esimesele reale kirjutada "mode 0") on toetatud kujud:
	* [argumentvektor] 0
	* [argumentvektor] -

*Input is given as argument vectors with their respective values in the truth table.*
* *Supported formatting in ones' mode:*
	* *[argument vector] 1*
	* *[argument vector] -*
* *Supported formatting in zeros' mode (first line must be "mode 0"):*
	* *[argument vector] 0*
	* *[argument vector] -*

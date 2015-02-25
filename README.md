MaidSafe-CRUX
=============

Work-in-progress.

CRUX (Connected Reliable Udp eXchange) is a reliable UDP protocol.

Maidsafe-CRUX is an implementation of CRUX for [Boost.Asio](http://www.boost.org/doc/libs/release/libs/asio/).

<p align="center"><small><b><u>Jenkins CI status (blue is pass, yellow is failing unit tests, red is build failure):</u></b></small></p>

<center>
<table id="configuration-matrix" width="100%" border="1">
  <tr>
    <td class="matrix-leftcolumn" rowspan="1" valign="top">g++-4.7</td>
  </tr>
  <tr>
    <td class="matrix-leftcolumn" rowspan="1" valign="top">g++-4.8</td><td class="matrix-cell"><div><a class="model-link inside" href="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=g++-4.8,LINKTYPE=shared,label=linux64-gcc-clang/"><img alt="Success" src="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=g++-4.8,LINKTYPE=shared,label=linux64-gcc-clang/badge/icon" tooltip="Success" align="top"/></a> linux64-gcc-clang</div></td>
  </tr>
  <tr>
    <td class="matrix-leftcolumn" rowspan="1" valign="top">g++-4.9</td><td class="matrix-cell"><div><a class="model-link inside" href="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=g++-4.9,LINKTYPE=shared,label=arm-gcc-clang/"><img alt="Success" src="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=g++-4.9,LINKTYPE=shared,label=arm-gcc-clang/badge/icon" tooltip="Success" align="top"/></a> arm-gcc-clang</div></td><td class="matrix-cell"><div><a class="model-link inside" href="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=g++-4.9,LINKTYPE=shared,label=linux64-gcc-clang/"><img alt="Success" src="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=g++-4.9,LINKTYPE=shared,label=linux64-gcc-clang/badge/icon" tooltip="Success" align="top"/></a> linux64-gcc-clang</div></td>
  </tr>
  <tr>
    <td class="matrix-leftcolumn" rowspan="1" valign="top">clang++-3.2</td>
  </tr>
  <tr>
    <td class="matrix-leftcolumn" rowspan="1" valign="top">clang++-3.3</td><td class="matrix-cell"><div><a class="model-link inside" href="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=clang++-3.3,LINKTYPE=shared,label=freebsd10-clang3.3/"><img alt="Unstable" src="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=clang++-3.3,LINKTYPE=shared,label=freebsd10-clang3.3/badge/icon" tooltip="Unstable" align="top"/></a> freebsd10-clang3.3</div></td>
  </tr>
  <tr>
    <td class="matrix-leftcolumn" rowspan="1" valign="top">clang++-3.4</td>
  </tr>
  <tr>
    <td class="matrix-leftcolumn" rowspan="1" valign="top">clang++-3.5</td><td class="matrix-cell"><div><a class="model-link inside" href="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=clang++-3.5,LINKTYPE=shared,label=arm-gcc-clang/"><img alt="Success" src="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=clang++-3.5,LINKTYPE=shared,label=arm-gcc-clang/badge/icon" tooltip="Success" align="top"/></a> arm-gcc-clang</div></td><td class="matrix-cell"><div><a class="model-link inside" href="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=clang++-3.5,LINKTYPE=shared,label=linux64-gcc-clang/"><img alt="Success" src="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=clang++-3.5,LINKTYPE=shared,label=linux64-gcc-clang/badge/icon" tooltip="Success" align="top"/></a> linux64-gcc-clang</div></td>
  </tr>
  <tr>
    <td class="matrix-leftcolumn" rowspan="1" valign="top">msvc-12.0</td><td class="matrix-cell"><div><a class="model-link inside" href="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=msvc-12.0,LINKTYPE=shared,label=win8-msvc-mingw/"><img alt="Unstable" src="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=msvc-12.0,LINKTYPE=shared,label=win8-msvc-mingw/badge/icon" tooltip="Unstable" align="top"/></a> win8-msvc-mingw</div></td>
  </tr>
  <tr>
    <td class="matrix-leftcolumn" rowspan="1" valign="top">msvc-14.0</td><td class="matrix-cell"><div><a class="model-link inside" href="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=msvc-14.0,LINKTYPE=shared,label=win8-msvc-mingw/"><img alt="Unstable" src="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=msvc-14.0,LINKTYPE=shared,label=win8-msvc-mingw/badge/icon" tooltip="Unstable" align="top"/></a> win8-msvc-mingw</div></td>
  </tr>
  <tr>
    <td class="matrix-leftcolumn" rowspan="1" valign="top">mingw64</td><td class="matrix-cell"><div><a class="model-link inside" href="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=mingw64,LINKTYPE=shared,label=win8-msvc-mingw/"><img alt="Unstable" src="https://ci.nedprod.com/job/Maidsafe%20CRUX/CPPSTD=c++11,CXX=mingw64,LINKTYPE=shared,label=win8-msvc-mingw/badge/icon" tooltip="Unstable" align="top"/></a> win8-msvc-mingw</div></td>
  </tr>
</table>
</center>


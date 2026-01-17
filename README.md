<a name="readme-top"></a>

<!-- PROJECT SHIELDS -->
<!--
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]
-->

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <h3 align="center">cssoptim</h3>

  <p align="center">
    A high-performance CSS optimizer and unused style remover.
    <br />
    <a href="https://github.com/antonialoy/cssoptim"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/antonialoy/cssoptim">View Demo</a>
    ·
    <a href="https://github.com/antonialoy/cssoptim/issues">Report Bug</a>
    ·
    <a href="https://github.com/antonialoy/cssoptim/issues">Request Feature</a>
  </p>
</div>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## About The Project

`cssoptim` is a command-line tool designed to reduce the size of CSS files by identifying and removing unused styles. It scans your HTML and JavaScript files to determine which classes, IDs, and tags are actually in use, and then prunes the CSS accordingly.

Features:
* Fast and efficient scanning of HTML and JS.
* Multiple optimization modes (Safe, Conservative, Strict).
* Deep integration with Lexbor for robust CSS/HTML parsing.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Built With

This project relies on the following major libraries:

* [Lexbor](https://lexbor.com/) - A fast HTML/CSS parsing and manipulation library.
* [Unity](https://github.com/ThrowTheSwitch/Unity) - Unit testing framework for C.
* [argparse](https://github.com/cofyc/argparse) - Command-line argument parsing library.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

To get a local copy up and running, follow these simple steps.

### Prerequisites

* A C compiler (GCC or Clang)
* `make`
* `liblexbor-dev` installed on your system.

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/antonialoy/cssoptim.git
   ```
2. Build the project
   ```sh
   make
   ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- USAGE EXAMPLES -->
## Usage

Run `cssoptim` with your CSS files and the HTML/JS files to scan.

```sh
./build/cssoptim --css style.css --html index.html -o optimized.css
```

For more options, run:
```sh
./build/cssoptim --help
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTACT -->
## Contact

Antoni Aloy Torrens - [your_twitter](https://twitter.com/your_username) - email@example.com

Project Link: [https://github.com/antonialoy/cssoptim](https://github.com/antonialoy/cssoptim)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* [Best-README-Template](https://github.com/othneildrew/Best-README-Template)
* [Choose an Open Source License](https://choosealicense.com)
* [Img Shields](https://shields.io)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
[contributors-shield]: https://img.shields.io/github/contributors/antonialoy/cssoptim.svg?style=for-the-badge
[contributors-url]: https://github.com/antonialoy/cssoptim/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/antonialoy/cssoptim.svg?style=for-the-badge
[forks-url]: https://github.com/antonialoy/cssoptim/network/members
[stars-shield]: https://img.shields.io/github/stars/antonialoy/cssoptim.svg?style=for-the-badge
[stars-url]: https://github.com/antonialoy/cssoptim/stargazers
[issues-shield]: https://img.shields.io/github/issues/antonialoy/cssoptim.svg?style=for-the-badge
[issues-url]: https://github.com/antonialoy/cssoptim/issues
[license-shield]: https://img.shields.io/github/license/antonialoy/cssoptim.svg?style=for-the-badge
[license-url]: https://github.com/antonialoy/cssoptim/blob/master/LICENSE
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/your_name

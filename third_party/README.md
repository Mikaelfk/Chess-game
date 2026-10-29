# Third-party components

CMake downloads and verifies these pinned upstream source archives at configure
time; there is no runtime network access:

| Component | Version | License | Use |
| --- | --- | --- | --- |
| Dear ImGui | 1.91.9b | MIT | Shared desktop/Android immediate-mode interface |
| SDL | 2.32.10 | zlib | Windows, Linux and Android rendering, input and lifecycle |

`cmake/Dependencies.cmake` records archive URLs and SHA-256 hashes. Their licenses
are included in installed desktop packages. For offline builds, pre-download the
sources and pass `FETCHCONTENT_SOURCE_DIR_IMGUI` and `FETCHCONTENT_SOURCE_DIR_SDL2`.
Android's Java source path requires SDL at `out/deps/SDL`; populate that path for
offline Gradle builds.

The following headers are vendored, with their original notices intact:

- `json.hpp`: nlohmann/json 3.12.0, MIT, from
  <https://github.com/nlohmann/json/releases/tag/v3.12.0>; see `json.LICENSE`.
- `stb_image.h`: stb commit `f0569113c93ad095470c54bf34a17b36646bbbb5`,
  MIT/public domain, PNG decoding; see `stb.LICENSE`.

Piece images are antialiased renders of Colin M. Burnett's GPLv2+ SVG artwork.
Original vector sources, provenance, regeneration steps and the artwork license
are in [Assets/Pieces](../Assets/Pieces/README.md). Both desktop and Android
packages include the source SVGs and notices.

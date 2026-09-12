# Contributing Guidelines

## Code of Conduct

This project follows the
[Contributor Covenant 2.0](https://www.contributor-covenant.org/version/2/0/code_of_conduct/).

## Quick Start

```sh
make build   # configure and build
make test    # run the test suite
make lint    # format, tidy, and static analysis checks
```

Full workflow:

```sh
git clone https://github.com/aplt-cnt/libcco.git
cd libcco
make test
git checkout -b feat/my-feature
# edit files
make lint && make test
git commit -s -m "feat: add my feature"
git push origin feat/my-feature
```

Then open a pull request against `main`.

## DCO

All commits must include a `Signed-off-by` trailer, appended by running
`git commit -s`. The CI DCO check rejects any PR whose commits lack it.

## PR Review Checklist

- [ ] `make lint` zero warnings
- [ ] `make test` all green
- [ ] Coverage meets the standard
- [ ] No credential leakage
- [ ] `docs/api.md` updated if any public API symbol changed

## Vibecoding disclosure

The PR template asks contributors to declare one of three modes:

- **Full vibecoding** - AI generated the code, human reviewed before
  submitting.
- **AI-assisted** - human drove the design and structure, AI augmented.
- **Hand-written** - no AI involvement.

Check exactly one. This is a transparency signal for reviewers, not a
quality gate. It does not affect merge decisions and is not checked by
CI.

## API documentation gate

Any PR that adds, removes, or renames a public symbol in
`include/cnt/cco.h` MUST update `docs/api.md` in the same commit. CI
enforces this via `scripts/check-api-doc.sh`. See `docs/api.md` for the
current surface.

## Private headers

Headers under `src/internal/` are private and are not installed.
Public declarations live only in `include/cnt/cco.h`. Do not add a
`cco_p.h` under `include/`.

## Language and formatting

- C11, no GNU extensions (`CMAKE_C_EXTENSIONS OFF`).
- clang-format is enforced via `.clang-format`. The style is Google-based
  with Allman braces and 4-space indent. See
  `adr/0001-clang-format-allman-over-appendix.md` for the reasoning.
- Commit messages follow Conventional Commits 1.0.0.

# Contributing to RTOS Task Manager

Thank you for considering contributing! Every contribution — bug fix, feature, or doc improvement — is welcome. Please read this guide before opening a PR.

---

## 📋 Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [How to Contribute](#how-to-contribute)
4. [Coding Style](#coding-style)
5. [Commit Conventions](#commit-conventions)
6. [Pull Request Process](#pull-request-process)

---

## Code of Conduct

This project follows a standard Code of Conduct. Be respectful, constructive, and collaborative.

---

## Getting Started

1. **Fork** the repository
2. **Clone** your fork:
   ```bash
   git clone https://github.com/<your-username>/rtos-task-manager.git
   cd rtos-task-manager
   ```
3. Create a **feature branch**:
   ```bash
   git checkout -b feat/my-new-feature
   ```
4. Make your changes and **test** locally (QEMU)
5. Open a **Pull Request** against `main`

---

## How to Contribute

### 🐛 Reporting Bugs
- Use the **Bug Report** issue template
- Include: steps to reproduce, expected vs actual behaviour, and your toolchain version

### 💡 Suggesting Features
- Use the **Feature Request** issue template
- Explain the motivation and any design considerations

### 🔧 Code Contributions
- Fix a bug listed in [Issues](https://github.com/pawanct08/rtos-task-manager/issues)
- Implement a planned module (`uart_cli.c`, `autosar_os.c`, `main.c`, `bsp/`)
- Improve unit test coverage
- Improve documentation

---

## Coding Style

This project follows **MISRA-C:2012** guidelines where practical and uses the FreeRTOS naming conventions.

- **Indentation**: 4 spaces (no tabs)
- **Line length**: ≤ 100 characters
- **Naming**:
  - Types: `PascalCase_t` (e.g. `TaskInfo_t`)
  - Functions: `Module_Function()` (e.g. `TaskManager_Init()`)
  - Macros / constants: `UPPER_SNAKE_CASE`
  - Local variables: `camelCase`
- **Headers**: Every `.c` file must have a corresponding `.h`
- **Comments**: Use Doxygen-style `/** @brief ... */` for all public API
- **No dynamic allocation** in interrupt context; use `MemPool_Alloc()` instead of `malloc()`

---

## Commit Conventions

Use [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <short description>

[optional body]
[optional footer]
```

Types: `feat`, `fix`, `docs`, `refactor`, `test`, `chore`, `ci`

Examples:
```
feat(mutex_guard): add priority inheritance on deadlock recovery
fix(mem_pool): prevent double-free corruption in ISR context
docs(readme): update QEMU launch instructions
```

---

## Pull Request Process

1. Ensure CI passes (build + static analysis)
2. Update `README.md` if you add a new module or change the API
3. Add/update Doxygen comments for any public API changes
4. Request a review — PRs are merged after at least **1 approval**

---

Thank you for helping make this project better! 🚀

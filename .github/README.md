# GitHub Configuration Directory

This directory contains GitHub-specific configuration files, including GitHub Actions workflows.

## Structure

```
.github/
└── workflows/
    └── ci.yml    # Main CI/CD pipeline
```

## Workflows

### ci.yml - Continuous Integration Pipeline

Comprehensive CI/CD pipeline that runs on push and pull requests.

**Features:**
- Multi-platform builds (Ubuntu, macOS, Windows)
- Multiple compiler support (GCC, Clang)
- Security scanning with CodeQL
- Test execution and coverage reporting
- Performance benchmarking
- Docker image building and pushing
- Release artifact creation

**Job Structure:**

1. **Build and Test Matrix**
   - Runs on: Ubuntu 22.04, Ubuntu 20.04, macOS 12, Windows 2022
   - Compilers: gcc-11, gcc-10, clang-14, clang-13
   - Build types: Debug, Release
   - Executes all tests

2. **Security Scanning**
   - CodeQL analysis for C++ code
   - Dependency vulnerability scanning
   - Security policy compliance

3. **Performance Testing**
   - Runs performance benchmarks
   - Compares against baseline
   - Uploads results as artifacts

4. **Docker Build**
   - Multi-stage Docker builds
   - Pushes to GitHub Container Registry
   - Tags with version and commit SHA

5. **Documentation**
   - Generates Doxygen documentation
   - Deploys to GitHub Pages

## Adding New Workflows

When creating new workflows:

1. Use meaningful job and step names
2. Utilize matrix builds for multiple configurations
3. Cache dependencies to speed up builds
4. Use GitHub Secrets for sensitive data
5. Add status badges to README
6. Consider workflow reusability

## Secrets Required

Configure these secrets in your repository settings:
- `GITHUB_TOKEN`: Automatically provided by GitHub
- `DOCKER_REGISTRY_TOKEN`: For pushing Docker images (if using external registry)
- `CODECOV_TOKEN`: For uploading coverage reports (optional)

## Branch Protection

Recommended branch protection rules:
- Require PR reviews before merging
- Require status checks to pass (CI build)
- Require branches to be up to date
- Include administrators in restrictions
- Require signed commits (optional)

## GitHub Apps and Integrations

Consider adding:
- **Dependabot**: Automated dependency updates
- **CodeQL**: Advanced security analysis
- **Codecov**: Code coverage tracking
- **SonarCloud**: Code quality analysis
- **Renovate**: Dependency management

## Issue and PR Templates

Create templates in `.github/ISSUE_TEMPLATE/` and `.github/PULL_REQUEST_TEMPLATE/`:
- Bug report template
- Feature request template
- Pull request template
- Security vulnerability template
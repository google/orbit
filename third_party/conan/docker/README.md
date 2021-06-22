# How to make changes to a docker image

## Introduction

Our docker image names need to match the CI's job names since
they are automatically chosen based on the job name.

For build jobs the job name also needs to match the Conan profile.

Example: Our Windows job is called `msvc2019_relwithdebinfo` as is the
Conan profile. The matching Docker image in our Docker registry is called
`gcr.io/orbitprofiler/msvc2019_relwithdebinfo`.

`build_containers.sh` contains a table which maps from Docker image names
to Dockerfile names. That's needed because multiple build jobs (Conan profiles)
share the same Dockerfile. For example `clang9_release` and `clang9_debug` use
the same Docker image definition.

## Step-By-Step

1. Determine the correct Dockerfile by checking the table in `build_containers.sh`.
2. Make your changes to the correct Dockerfile.
3. Call `./build_containers.sh <<jobname(s)>>`. This will automatically update the tag
   in `tags.sh`. You will need to add this automatic change to your commit later.
4. Call `./upload_containers.sh <<jobname(s)>>`. This will automatically update the digest
   list in `digests.sh`. These are hashes which uniquely identify an image version.
   That allows to be sure that we always use exactly the same image. You will need to add
   this automatic change to your commit later.
5. Commit your changes from the Dockerfile, `tags.sh`, and `digests.sh`.
6. Create a (draft) PR and see if your new image works on the CI.
7. Submit.

Note: When making a change to a Dockerfile, ensure that you always update all corresponding
      images. That's particurlarly important for build images. For example, after a change to
      `Dockerfile.msvc2019`, the images `msvc2019_release`, `msvc2019_relwithdebinfo`, and
      `msvc2019_debug` need to be updated.

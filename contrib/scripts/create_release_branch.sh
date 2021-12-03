#!/bin/bash
# copyright (c) 2021 the orbit authors. all rights reserved.
# use of this source code is governed by a bsd-style license that can be
# found in the license file.

set -euo pipefail

if [[ ! $1 =~ ^1\.([[:digit:]]{2,3})$ ]]; then
  echo "Please provide a valid Orbit version number as the first positional argument. Example: 1.76" > /dev/stderr
  exit 1
fi

readonly ORBIT_VERSION="$1"
readonly ORBIT_MINOR_VERSION=${BASH_REMATCH[1]}
readonly ORBIT_NEXT_VERSION="1.$(($ORBIT_MINOR_VERSION + 1))"

readonly REPO_ROOT="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/../.." &> /dev/null && pwd )"

pushd ${REPO_ROOT} >/dev/null

if ! git rev-parse 2>/dev/null; then
  echo "It seems this script is not part of a valid Git repository. Can't continue..." >/dev/stderr
  exit 1
fi

# Should match both SSH and HTTPS URLs
readonly UPSTREAM_URL_REGEX="github\.com.google/orbit"

if ! git remote -v | grep -qE ${UPSTREAM_URL_REGEX};  then
  echo "Couldn't find the google/orbit Github repo in the remotes. Can't continue..." >/dev/stderr
  exit 1
fi

readonly UPSTREAM_REMOTE_NAME="$(git remote -v | grep -E ${UPSTREAM_URL_REGEX} | head -n1 | cut -d$'\t' -f1)"
readonly UPSTREAM_MAIN_BRANCH_NAME="main"

git fetch ${UPSTREAM_REMOTE_NAME}

readonly UPSTREAM_RELEASE_BRANCH_NAME="release/${ORBIT_VERSION}"

if git branch --remote --list | grep -q "${UPSTREAM_REMOTE_NAME}/${UPSTREAM_RELEASE_BRANCH_NAME}"; then
  echo "An upstream branch for release ${ORBIT_VERSION} already exists." >/dev/stderr
  echo "If you really want to continue, please delete the upstream branch first." >/dev/stderr
  exit 1
fi

readonly CURRENT_BRANCH_NAME="$(git symbolic-ref --short HEAD)"

readonly PREVIOUS_STASH_TIP="$(git rev-parse -q --verify refs/stash)"
git stash -m "Worktree state saved by create_release_branch.sh"
readonly CURRENT_STASH_TIP="$(git rev-parse -q --verify refs/stash)"

function restore_git_state {
  git checkout -q ${CURRENT_BRANCH_NAME}

  if [[ ! $CURRENT_STASH_TIP == $PREVIOUS_STASH_TIP ]]; then
    git stash pop -q --index
  fi
}

trap restore_git_state EXIT

### Checking if the next development cycle has already been started
readonly NEXT_DEV_CYCLE_COMMIT_MSG="Beginning of the ${ORBIT_NEXT_VERSION} development cycle"
readonly NEXT_DEV_CYCLE_COMMIT_ID="$(git log --grep="${NEXT_DEV_CYCLE_COMMIT_MSG}" -n 1 --pretty=format:"%H" ${UPSTREAM_REMOTE_NAME}/${UPSTREAM_MAIN_BRANCH_NAME})"

if [[ -z ${NEXT_DEV_CYCLE_COMMIT_ID} ]]; then
  # If not we will create a PR with an empty commit marking the beginning
  echo -e "\n\nIt seems the ${ORBIT_NEXT_VERSION} development cycle has not been started yet." > /dev/stderr
  echo "Taking care of that now..." > /dev/stderr

  readonly START_DEV_CYCLE_BRANCH_NAME="script/create_release_branch/start_next_dev_cycle"
  git branch "${START_DEV_CYCLE_BRANCH_NAME}" ${UPSTREAM_REMOTE_NAME}/${UPSTREAM_MAIN_BRANCH_NAME}

  function delete_start_dev_cycle_branch {
    git branch -D "${START_DEV_CYCLE_BRANCH_NAME}"
  }

  trap "restore_git_state; delete_start_dev_cycle_branch" EXIT

  git switch "${START_DEV_CYCLE_BRANCH_NAME}"

  git commit --quiet --allow-empty -m "${NEXT_DEV_CYCLE_COMMIT_MSG}"

  # We create the branch NOT on the personal fork which avoids figuring out another remote.
  git push ${UPSTREAM_REMOTE_NAME} ${START_DEV_CYCLE_BRANCH_NAME}:${START_DEV_CYCLE_BRANCH_NAME}

  echo -e "\n\nThe script just created a branch '${START_DEV_CYCLE_BRANCH_NAME}' on '${UPSTREAM_REMOTE_NAME}' containing an empty commit" > /dev/stderr
  echo "that marks the beginning of the ${ORBIT_NEXT_VERSION} development cycle." > /dev/stderr
  echo "Please create a PR targeting '${UPSTREAM_MAIN_BRANCH_NAME}' and" > /dev/stderr
  echo "please ensure that the merged commit will contain the unchanged commit message." > /dev/stderr
  echo "That's particularly important when squashing!" > /dev/stderr
  echo "When the PR is merged, please call this script again!" > /dev/stderr

  exit 0
fi

echo -e "\n\nThe beginning of the ${ORBIT_NEXT_VERSION} development cycle has been found." > /dev/stderr
echo "Next the script will tag the development cycle and then move on to creating the release branch." > /dev/stderr

readonly DEV_CYCLE_TAG_NAME="${ORBIT_NEXT_VERSION}dev"
git tag -a -m "Beginning of ${ORBIT_NEXT_VERSION} development cycle" ${DEV_CYCLE_TAG_NAME} ${NEXT_DEV_CYCLE_COMMIT_ID}

function delete_local_dev_tag {
  git tag -d ${DEV_CYCLE_TAG_NAME}
}

trap "restore_git_state; delete_local_dev_tag" EXIT

echo -e "\n\nPushing ${DEV_CYCLE_TAG_NAME} tag..."
git push ${UPSTREAM_REMOTE_NAME} ${DEV_CYCLE_TAG_NAME}

readonly LOCAL_RELEASE_BRANCH_NAME="script/create_release_branch/release_branch"
# We branch from one commit BEFORE the beginning of the next development cycle
git branch ${LOCAL_RELEASE_BRANCH_NAME} ${NEXT_DEV_CYCLE_COMMIT_ID}~1

function delete_local_release_branch {
  git branch -D "${LOCAL_RELEASE_BRANCH_NAME}"
}

trap "restore_git_state; delete_local_dev_tag; delete_local_release_branch" EXIT

git switch ${LOCAL_RELEASE_BRANCH_NAME}

git commit --quiet --allow-empty -m "Release branch for Orbit ${ORBIT_VERSION}"

readonly RC_TAG_NAME="${ORBIT_VERSION}rc"
git tag -a -m "Release candidate for Orbit ${ORBIT_VERSION}" ${RC_TAG_NAME}

function delete_local_rc_tag {
  git tag -d ${RC_TAG_NAME}
}

trap "restore_git_state; delete_local_dev_tag; delete_local_release_branch; delete_local_rc_tag" EXIT

git commit --quiet --allow-empty -m "First release candidate for Orbit ${ORBIT_VERSION}"

echo -e "\n\nPushing ${UPSTREAM_RELEASE_BRANCH_NAME} branch..."
git push ${UPSTREAM_REMOTE_NAME} ${LOCAL_RELEASE_BRANCH_NAME}:${UPSTREAM_RELEASE_BRANCH_NAME}

echo -e "\n\nPushing ${RC_TAG_NAME} tag..."
git push ${UPSTREAM_REMOTE_NAME} ${RC_TAG_NAME}

echo -e "\n\nAll done."
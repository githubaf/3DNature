# Create toolchain_commit_short.txt (sortiert und nur kurze Hashes)
# aehnlich "af_install_gcc"
# AF, 30.Jan 2024

BRANCH=$1 # 'master' or 'alt-abiv0' given on command-line
CURRENTDIR="$(pwd)"
PREFIX='/home/developer/Desktop/SelcoGit/AROS'

ALLOWED_BRANCHES=("master" "alt-abiv0")
if [[ ! " ${ALLOWED_BRANCHES[@]} " =~ " $1 " ]]; then
   echo "Unknown branch <$1>!" 
   echo "List of allowed branches:"
   for ALLOWED_BRANCH in "${ALLOWED_BRANCHES[@]}"; do
      echo "- $ALLOWED_BRANCH"
   done
   exit 1
fi

cd "$PREFIX"

GITURL="$(git remote -v | awk '/fetch/{print $2}')"

# Create toolchain_commit.txt
echo $GITURL > /tmp/toolchain_commit.txt
git log "$BRANCH" -1 --pretty=format:"$(basename "$(pwd)"): %h %cd %aN: %s" --date=short  >>/tmp/toolchain_commit.txt
echo "" >> /tmp/toolchain_commit.txt

AROS_GCC_DATE="$(cat /tmp/toolchain_commit.txt | awk '/AROS:/{print $3}')"  # juengstes Commit-Datum

echo "Toolchain:$AROS_GCC_DATE" #>/tmp/toolchain_commit_short.txt
cat /tmp/toolchain_commit.txt  | awk 'NR == 1; NR > 1 {print $0 | "sort -n"}' | awk '{print $1$2}' #>>"$PREFIX/bin/toolchain_commit_short.txt"


cd "$CURRENTDIR"

#### How to import project from BitBucket

```
cd /home/your-bitbucket-directory
git remote rename origin bitbucket
git remote add origin https://github.com/your/new-git-repo.git
git push origin master

git remove rm bitbucket     // if you want
```


#### How to remove commits from remote repository

```
git log --pretty=oneline --abbrev-commit
```
press 'q' to exist the log if it's too long

```
git rebase -i HEAD~4
```
When you intend to remove the last 3 commits; vim editor will automatically open; delete lines that corresponds to the commits you want to remove
```
git push origin +master
```

Refer to [this blog](https://ncona.com/2011/07/how-to-delete-a-commit-in-git-local-and-remote/)

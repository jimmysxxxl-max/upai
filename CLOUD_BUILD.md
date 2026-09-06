# Build the DLL without installing anything on your PC

This project includes a GitHub Actions workflow that builds the Windows F4SE DLL on GitHub's own Windows machine.

## What you need

- A free GitHub account.
- A web browser.
- No XMake, Visual Studio, compiler, or admin rights on your PC.

## Steps

1. Go to GitHub and create a new empty repository. Public or private is fine.
2. Extract this ZIP on your PC.
3. Upload **the contents inside the `UniquePlayerRedirector` folder** to that repository. Make sure `.github/workflows/build-dll.yml` is included.
4. Open the repository's **Actions** tab.
5. Select **Build UniquePlayerRedirector DLL**.
6. Click **Run workflow**, then **Run workflow** again.
7. When the run finishes, open it and download the artifact named **UniquePlayerRedirector-built**.
8. Inside it is `UniquePlayerRedirector-built.zip`; extract that into your mod manager or Fallout 4 folder as appropriate.

The final mod layout is:

```text
Data/
  F4SE/
    Plugins/
      UniquePlayerRedirector.dll
      UniquePlayerRedirector.ini
```

If the build turns red, open the failed run and copy the error text from the **Build** step back into ChatGPT. No local compiler troubleshooting is required.

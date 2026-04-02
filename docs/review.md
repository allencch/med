# Review Checklist

## Threading
- No UI blocking calls
- No mutex between UI and worker

## Architecture
- Core has zero Qt dependency
- Worker is the only bridge

## Code Smells
- Direct method calls across threads
- Mixed responsibilities

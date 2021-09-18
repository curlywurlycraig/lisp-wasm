#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <parse.h>
#include <emit.h>

#define ERROR(...) fprintf(stderr, "ERROR: " __VA_ARGS__)

struct CliOpts {
    const char* filename;
};

static bool parseCliArgs(int argc, char* argv[], struct CliOpts* cli)
{
  int opt = 0;

  while ((opt = getopt(argc, argv, "c:")) != -1) {
    switch (opt) {
      case 'c': {
        if (cli->filename) {
          ERROR("provide single file to compile\n");
          return false;
        }

        cli->filename = optarg;
      }
      break;

      default: {
        return false;
      }
      break;
    }
  }

  if (!cli->filename) {
      ERROR("provide single file to compile\n");
      return false;
  }

  return true;
}

int main(int argc, char** argv) {
  // Read filename from args
  char* filename = argv[1];
  (void)filename;
  (void)argc;

  struct CliOpts cli = {0};

  if (!parseCliArgs(argc, argv, &cli)) {
    return EXIT_FAILURE;
  }

  // Read file
  FILE* fp = fopen(cli.filename, "rb");

  if (!fp) {
    ERROR("failed to read file '%s'\n", cli.filename);
    return EXIT_FAILURE;
  }

  fseek(fp, 0, SEEK_END);
  int length = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char* programBuffer = malloc(length);

  if (!programBuffer) {
    ERROR("couldn't allocate program buffer\n");
    return EXIT_FAILURE;
  }

  fread(programBuffer, length, 1, fp);
  fclose(fp);

  // Do simple compilation
  initTokenFinders();
  TokenizeResult* tokens = malloc(sizeof(TokenizeResult));
  tokenize(tokens, programBuffer);

  ParseInfo* parseInfo = malloc(sizeof(ParseInfo));
  parseInfo->tokenIndex = 0;
  parseInfo->didFail = 0;
  parseInfo->reachedEnd = 0;
  parseInfo->tokenizeResult = tokens;
  parseInfo->raw = programBuffer;

  Program* parsedProgram = program(parseInfo);

  if (parseInfo->didFail) {
    ERROR("Failed to parse a program\n");
    return EXIT_FAILURE;
  }

  emit(parsedProgram);

  return 0;
}

#include "config.h"
#include "../constants.h"
#include "../util/error_handling.h"

namespace config {
map_entry config;

int initialize() {
  validator::reset_errors(); validator::validate("", default_config, default_constraint_passdown);
  if (validator::errors) {
    logger::config.note(std::to_string(validator::errors) + " error(s) generated in total when validating Default Config.");
    logger::config.note("Choose any of these methods to contact Daiari (please describe your situation and provide this file):");
    logger::config.note(AUTHOR_CONTACT);
    return 1;
  }

  loader::reset_errors(); loader::load_ini_from_file(config_path, config);
  if (loader::errors || loader::warns) {
    logger::config.note(std::to_string(loader::errors) + " error(s) and " + std::to_string(loader::warns) + " warning(s) generated in total when loading Config.");
    logger::config.note("You should check mentioned lines in '" + config_path + "'.");
    logger::config.note("If you have no clue, choose any of these methods to contact Daiari (please describe your situation and provide this file):");
    logger::config.note(AUTHOR_CONTACT);
  }
  if (loader::errors) return 2;
  else if (loader::warns) return 3;

  user_validator::reset_errors(); user_validator::validate("", config, default_constraint_passdown);
  if (user_validator::warns) {
    logger::config.note(std::to_string(user_validator::warns) + " warning(s) generated in total when validating Config.");
    logger::config.note("You should check mentioned keys in '" + config_path + "'.");
    logger::config.note("If you have no clue, choose any of these methods to contact Daiari (please describe your situation and provide this file):");
    logger::config.note(AUTHOR_CONTACT);
    return 4;
  }
  return 0;
}

}

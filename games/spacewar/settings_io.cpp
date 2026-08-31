#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

#include "app/options.h"
#include "sim/settings.h"

namespace {

constexpr double kSaveRounding = 1000.0;

// Snap to 3 decimals: writes the clean value (0.35, not 0.349999...) and
// stops repeated save/load from drifting the float downward.
void WriteNum(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w, float v) {
  w.Double(std::round(v * kSaveRounding) / kSaveRounding);
}

}  // namespace

namespace spacewar::sim {

using namespace se;

namespace {

// Guarded reads: each checks presence + type before touching dst, so a typo in
// the file fails the load instead of asserting.
bool GetF(const rapidjson::Value& o, const char* k, float& dst) {
  if (!o.HasMember(k) || !o[k].IsNumber()) return false;
  dst = o[k].GetFloat();
  return true;
}

bool GetI(const rapidjson::Value& o, const char* k, int& dst) {
  if (!o.HasMember(k) || !o[k].IsInt()) return false;
  dst = o[k].GetInt();
  return true;
}

bool GetU(const rapidjson::Value& o, const char* k, std::uint32_t& dst) {
  if (!o.HasMember(k) || !o[k].IsUint()) return false;
  dst = o[k].GetUint();
  return true;
}

bool GetVec2(const rapidjson::Value& o, const char* k, Vec2& dst) {
  if (!o.HasMember(k)) return false;
  const rapidjson::Value& a = o[k];
  if (!a.IsArray() || a.Size() != 2 || !a[0].IsNumber() || !a[1].IsNumber())
    return false;
  dst = {a[0].GetFloat(), a[1].GetFloat()};
  return true;
}

const rapidjson::Value* GetObj(const rapidjson::Value& o, const char* k) {
  if (!o.HasMember(k) || !o[k].IsObject()) return nullptr;
  return &o[k];
}

}  // namespace

bool LoadSimSettings(const char* path, SimSettings& out) {
  std::ifstream file(path);
  if (!file) return false;

  std::stringstream ss;
  ss << file.rdbuf();
  const std::string text = ss.str();

  rapidjson::Document d;
  d.Parse(text.c_str());
  if (d.HasParseError() || !d.IsObject()) return false;  // malformed JSON

  const rapidjson::Value* ship = GetObj(d, "ship");
  const rapidjson::Value* torpedo = GetObj(d, "torpedo");
  const rapidjson::Value* star = GetObj(d, "star");
  const rapidjson::Value* spawn = GetObj(d, "spawn");
  const rapidjson::Value* match = GetObj(d, "match");
  const rapidjson::Value* scoring = GetObj(d, "scoring");
  const rapidjson::Value* hyper = GetObj(d, "hyperspace");
  if (!ship || !torpedo || !star || !spawn || !match || !scoring || !hyper)
    return false;

  // Parse into a temp; commit to out only if every field reads cleanly. `&=`
  // doesn't short-circuit, so ok goes false if any single read failed.
  SimSettings t{};
  bool ok = true;

  ok &= GetF(*ship, "turn_rate", t.ship.turn_rate);
  ok &= GetF(*ship, "thrust_accel", t.ship.thrust_accel);
  ok &= GetF(*ship, "max_speed", t.ship.max_speed);
  ok &= GetF(*ship, "fire_cooldown", t.ship.fire_cooldown);
  ok &= GetF(*ship, "muzzle_offset", t.ship.muzzle_offset);
  ok &= GetF(*ship, "collision_radius", t.ship.collision_radius);

  ok &= GetF(*torpedo, "life_sec", t.torpedo.life_sec);
  ok &= GetF(*torpedo, "launch_speed", t.torpedo.launch_speed);
  ok &= GetF(*torpedo, "max_speed", t.torpedo.max_speed);
  ok &= GetF(*torpedo, "recoil", t.torpedo.recoil);
  ok &= GetF(*torpedo, "collision_radius", t.torpedo.collision_radius);

  ok &= GetF(*star, "gravity", t.star.gravity);
  ok &= GetF(*star, "softening", t.star.softening);
  ok &= GetF(*star, "radius", t.star.radius);

  ok &= GetVec2(*spawn, "needle_start", t.spawn.needle_start);
  ok &= GetVec2(*spawn, "wedge_start", t.spawn.wedge_start);
  ok &= GetF(*spawn, "orbit_speed", t.spawn.orbit_speed);

  ok &= GetF(*match, "respawn_sec", t.match.respawn_sec);
  ok &= GetF(*match, "match_sec", t.match.match_sec);
  ok &= GetF(*match, "settle_sec", t.match.settle_sec);

  ok &= GetI(*scoring, "fire_cost", t.scoring.fire_cost);
  ok &= GetI(*scoring, "kill", t.scoring.kill);
  ok &= GetI(*scoring, "self_kill", t.scoring.self_kill);
  ok &= GetI(*scoring, "star_suicide", t.scoring.star_suicide);

  ok &= GetI(*hyper, "base_fail_percent", t.hyperspace.base_fail_percent);
  ok &= GetI(*hyper, "fail_step_percent", t.hyperspace.fail_step_percent);
  ok &= GetI(*hyper, "suicide_penalty", t.hyperspace.suicide_penalty);
  ok &= GetF(*hyper, "cooldown_sec", t.hyperspace.cooldown_sec);
  ok &= GetF(*hyper, "anim_duration_sec", t.hyperspace.anim_duration_sec);
  ok &= GetF(*hyper, "ship_spin_rate", t.hyperspace.ship_spin_rate);

  ok &= GetU(d, "rng_seed", t.rng_seed);

  if (!ok) return false;
  out = t;
  return true;
}

bool SaveSimSettings(const char* path, const SimSettings& in) {
  rapidjson::StringBuffer buf;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> w(buf);
  auto num = [&](float v) { WriteNum(w, v); };
  auto vec2 = [&](const Vec2& v) {
    w.StartArray();
    num(v.x);
    num(v.y);
    w.EndArray();
  };

  w.StartObject();

  w.Key("ship");
  w.StartObject();
  w.Key("turn_rate");
  num(in.ship.turn_rate);
  w.Key("thrust_accel");
  num(in.ship.thrust_accel);
  w.Key("max_speed");
  num(in.ship.max_speed);
  w.Key("fire_cooldown");
  num(in.ship.fire_cooldown);
  w.Key("muzzle_offset");
  num(in.ship.muzzle_offset);
  w.Key("collision_radius");
  num(in.ship.collision_radius);
  w.EndObject();

  w.Key("torpedo");
  w.StartObject();
  w.Key("life_sec");
  num(in.torpedo.life_sec);
  w.Key("launch_speed");
  num(in.torpedo.launch_speed);
  w.Key("max_speed");
  num(in.torpedo.max_speed);
  w.Key("recoil");
  num(in.torpedo.recoil);
  w.Key("collision_radius");
  num(in.torpedo.collision_radius);
  w.EndObject();

  w.Key("star");
  w.StartObject();
  w.Key("gravity");
  num(in.star.gravity);
  w.Key("softening");
  num(in.star.softening);
  w.Key("radius");
  num(in.star.radius);
  w.EndObject();

  w.Key("spawn");
  w.StartObject();
  w.Key("needle_start");
  vec2(in.spawn.needle_start);
  w.Key("wedge_start");
  vec2(in.spawn.wedge_start);
  w.Key("orbit_speed");
  num(in.spawn.orbit_speed);
  w.EndObject();

  w.Key("match");
  w.StartObject();
  w.Key("respawn_sec");
  num(in.match.respawn_sec);
  w.Key("match_sec");
  num(in.match.match_sec);
  w.Key("settle_sec");
  num(in.match.settle_sec);
  w.EndObject();

  w.Key("scoring");
  w.StartObject();
  w.Key("fire_cost");
  w.Int(in.scoring.fire_cost);
  w.Key("kill");
  w.Int(in.scoring.kill);
  w.Key("self_kill");
  w.Int(in.scoring.self_kill);
  w.Key("star_suicide");
  w.Int(in.scoring.star_suicide);
  w.EndObject();

  w.Key("hyperspace");
  w.StartObject();
  w.Key("base_fail_percent");
  w.Int(in.hyperspace.base_fail_percent);
  w.Key("fail_step_percent");
  w.Int(in.hyperspace.fail_step_percent);
  w.Key("suicide_penalty");
  w.Int(in.hyperspace.suicide_penalty);
  w.Key("cooldown_sec");
  num(in.hyperspace.cooldown_sec);
  w.Key("anim_duration_sec");
  num(in.hyperspace.anim_duration_sec);
  w.Key("ship_spin_rate");
  num(in.hyperspace.ship_spin_rate);
  w.EndObject();

  w.Key("rng_seed");
  w.Uint(in.rng_seed);

  w.EndObject();

  std::ofstream file(path);
  if (!file) return false;
  file << buf.GetString();
  return static_cast<bool>(file);
}

}  // namespace spacewar::sim

namespace spacewar::app {

bool LoadAppOptions(const char* path, AppOptions& out) {
  std::ifstream file(path);
  if (!file) return false;

  std::stringstream ss;
  ss << file.rdbuf();

  rapidjson::Document d;
  d.Parse(ss.str().c_str());
  if (d.HasParseError() || !d.IsObject()) return false;

  AppOptions t;
  if (d.HasMember("master_volume") && d["master_volume"].IsNumber())
    t.master_volume = d["master_volume"].GetFloat();
  if (d.HasMember("fullscreen") && d["fullscreen"].IsBool())
    t.fullscreen = d["fullscreen"].GetBool();
  out = t;
  return true;
}

bool SaveAppOptions(const char* path, const AppOptions& in) {
  rapidjson::StringBuffer buf;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> w(buf);
  auto num = [&](float v) { WriteNum(w, v); };
  w.StartObject();
  w.Key("master_volume");
  num(in.master_volume);
  w.Key("fullscreen");
  w.Bool(in.fullscreen);
  w.EndObject();

  std::ofstream file(path);
  if (!file) return false;
  file << buf.GetString();
  return static_cast<bool>(file);
}

}  // namespace spacewar::app

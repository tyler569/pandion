#include "minecraft.h"
#include "nbt.h"

static struct nbt_tag *make_overworld_element() {
	struct nbt_tag *tag = nbt_new_compound();

	nbt_add_byte_to_compound(tag, "piglin_safe", 0);
	nbt_add_byte_to_compound(tag, "natural", 1);
	nbt_add_float_to_compound(tag, "ambient_light", 0.0f);
	nbt_add_string_to_compound(
		tag, "infiniburn", "#minecraft:infiniburn_overworld");
	nbt_add_byte_to_compound(tag, "respawn_anchor_works", 0);
	nbt_add_byte_to_compound(tag, "has_skylight", 1);
	nbt_add_byte_to_compound(tag, "bed_works", 1);
	nbt_add_string_to_compound(tag, "effects", "minecraft:overworld");
	nbt_add_byte_to_compound(tag, "has_raids", 1);
	nbt_add_int_to_compound(tag, "min_y", -64);
	nbt_add_int_to_compound(tag, "height", 384);
	nbt_add_int_to_compound(tag, "logical_height", 384);
	nbt_add_double_to_compound(tag, "coordinate_scale", 1.0);
	nbt_add_byte_to_compound(tag, "ultrawarm", 0);
	nbt_add_byte_to_compound(tag, "has_ceiling", 0);

	return tag;
}

static struct nbt_tag *make_biome_effects() {
	struct nbt_tag *biome_effects = nbt_new_compound();

	nbt_add_int_to_compound(biome_effects, "sky_color", 7907327);
	nbt_add_int_to_compound(biome_effects, "water_fog_color", 329011);
	nbt_add_int_to_compound(biome_effects, "fog_color", 12638463);
	nbt_add_int_to_compound(biome_effects, "water_color", 4159204);

	return biome_effects;
}

static struct nbt_tag *make_plains_biome() {
	struct nbt_tag *plains = nbt_new_compound();

	nbt_add_string_to_compound(plains, "name", "minecraft:plains");
	nbt_add_int_to_compound(plains, "id", 0);

	struct nbt_tag *plains_element = nbt_new_compound();

	nbt_add_string_to_compound(plains_element, "precipitation", "rain");
	nbt_add_float_to_compound(plains_element, "depth", 0.125f);
	nbt_add_float_to_compound(plains_element, "temperature", 0.8f);
	nbt_add_float_to_compound(plains_element, "scale", 0.05f);
	nbt_add_float_to_compound(plains_element, "downfall", 0.4f);
	nbt_add_string_to_compound(plains_element, "category", "plains");

	nbt_add_compound_to_compound(
		plains_element, "effects", make_biome_effects());

	nbt_add_compound_to_compound(plains, "element", plains_element);

	return plains;
}

static struct nbt_tag *make_biome_registry() {
	struct nbt_tag *biome_registry = nbt_new_compound();

	nbt_add_string_to_compound(
		biome_registry, "type", "minecraft:worldgen/biome");

	struct nbt_tag *biomes = nbt_new_list(NBT_COMPOUND);

	nbt_add_compound_to_list(biomes, make_plains_biome());

	nbt_add_list_to_compound(biome_registry, "value", biomes);

	return biome_registry;
};

static struct nbt_tag *make_overworld() {
	struct nbt_tag *overworld = nbt_new_compound();

	nbt_add_string_to_compound(overworld, "name", "minecraft:overworld");
	nbt_add_int_to_compound(overworld, "id", 0);

	struct nbt_tag *overworld_element = make_overworld_element();

	nbt_add_compound_to_compound(overworld, "element", overworld_element);

	return overworld;
}

static struct nbt_tag *make_dimension_type_registry() {
	struct nbt_tag *dimension_type_registry = nbt_new_compound();

	nbt_add_string_to_compound(
		dimension_type_registry, "type", "minecraft:dimension_type");

	struct nbt_tag *dimensions = nbt_new_list(NBT_COMPOUND);
	nbt_add_compound_to_list(dimensions, make_overworld());
	nbt_add_list_to_compound(dimension_type_registry, "value", dimensions);

	return dimension_type_registry;
}

void init_server_state(struct server *s) {
	s->dimension_codec = nbt_new_compound();

	nbt_add_compound_to_compound(s->dimension_codec, "minecraft:dimension_type",
		make_dimension_type_registry());

	nbt_add_compound_to_compound(
		s->dimension_codec, "minecraft:worldgen/biome", make_biome_registry());

	FILE *ser_file = fopen("dimension_codec.nbt", "wb");
	nbt_write_to_stream(s->dimension_codec, ser_file);
	fclose(ser_file);

	s->dimension = make_overworld_element();
}
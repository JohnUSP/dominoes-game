# DoubleNineStandard

Status: agreed rules with unresolved details listed explicitly below.

This document records the rules selected for this project. It does not claim to
define a universal Cuban Domino ruleset. Alternative formats and optional rules
discussed for future variants are outside this specification.

## Terminology

- **Hand:** play using one deal, ending in domination, blocking, invalid pass,
  or timeout.
- **Match:** successive hands until a team reaches the target score.
- **Pip total:** the sum of both numbers on a tile, or the sum over a player's
  remaining tiles.
- **Opening:** the first tile played in a hand.
- **Null hand:** a blocked hand tied between rival teams, awarding no points.

## Players and teams

- Four players form two teams of two.
- Partners sit opposite each other. Turn order alternates teams.
- Players are supplied in playing order, `A1, B1, A2, B2`. Team A occupies
  indices zero and two; team B occupies indices one and three.
- Each team may have a custom name. Otherwise, its name is the two player names
  joined by ` & `, for example, `John & Kyle`.

## Tiles and dealing

- Use a double-nine set: all 55 distinct unordered pairs with values from zero
  through nine, inclusive.
- Each player receives ten tiles.
- The remaining fifteen tiles are unused for that hand.
- Tiles are never drawn during play.
- Each hand starts with a complete set available for shuffling and dealing.
- The exact dealing sequence has not yet been selected.

## Opening

- Select the first opening team randomly.
- Opening is initially assigned to the first player of that team (`A1` or `B1`).
- That player may open or transfer the opening to their partner. The partner
  may open or return it once; no further transfers are allowed.
- Opening transfer is a separate action from passing during ordinary play and
  does not count toward blocking. Transfers are unavailable after the first tile
  is played.
- The opening player may choose any tile in their hand; no double is required.
- After a decisive hand, the winning player has the opening choice for the next
  hand: open personally or offer opening to their partner.
- For a blocked hand with a winning team, the lowest-scoring player receives
  opening. If both partners tie for the lowest total, their team's first player
  (`A1` or `B1`) receives it. The same transfer-and-return rule applies.
- The partner may return the opening choice once. Repeated exchanges are not
  permitted.
- After a null hand, the team that opened that hand retains opening, initially
  assigned to its first player (`A1` or `B1`). The same transfer-and-return rule
  applies.
- After a timeout or invalid-pass loss, the winning team's first player (`A1`
  or `B1`) receives opening by default. The same transfer-and-return rule applies.

## Moves and passing

- Players act in seating order, alternating teams.
- Passing is an explicit player action, not an automatic action by the game.
- A player must pass when they have no legal move and may not pass when a legal
  move exists.
- Passing while a legal move exists immediately loses the hand for the passing
  player's team, with the same scoring consequence as a timeout.
- A player who has no legal move remains on the clock until they explicitly pass
  or the hand ends.
- A legal pass receives the same timing treatment as a tile play.
- Completing a legal tile play or pass advances the turn unless the hand ends.

For a tile play, the player selects a tile by its current hand index and chooses
the left or right board end. The player does not specify a matching number.
Placement is legal if either number on the tile matches the selected exposed
end. The engine orients the tile so that the matching number connects to that
end. The opening accepts any tile without a matching requirement.

The player may omit the board end. If neither end accepts the selected tile, the
move is rejected. If only one end accepts it, that end is chosen automatically.
If both ends accept it and the exposed board numbers are equal, the right end
is chosen for now. If both ends accept it but the exposed numbers differ, the
player must explicitly choose an end; omission is rejected. An explicitly
selected end is always respected and must match. A rejected move leaves the
turn, board, and hand unchanged. On opening, the board end may always be omitted.

For example, a tile `[7|9]` requires an explicit end when the board exposes
`9` on the left and `7` on the right. Against exposed ends `7` and `7`, the same
tile defaults to the right when no end is supplied.

## Hand completion and winner

### Domination

When a player plays their last tile, the hand ends immediately and their team
wins.

### Blocking

The hand is blocked when no player has a legal move.

Compare the four individual remaining pip totals; do not compare combined team
totals to determine the winner.

- If the minimum belongs to one player, that player's team wins.
- If multiple players share the minimum and all belong to the same team, that
  team wins.
- If the minimum is shared by players from rival teams, the hand is null.
- No secondary comparison of partners' totals is used in DoubleNineStandard.

Blocking is declared after four consecutive legal, explicit passes. A successful
tile play resets the consecutive-pass count. Rejected moves do not reset it.

### Timeout

If a player's personal time expires, the hand ends immediately and that player's
team loses. The opposing team wins.

## Scoring and match completion

- Both teams start the match with zero points.
- After a decisive hand, the winning team receives the sum of the two losing
  players' remaining pip totals.
- The winning team's remaining tiles do not contribute to the award.
- The same scoring rule applies to timeout and invalid-pass losses.
- A null hand awards no points to either team.
- Scores accumulate across hands.
- The match ends as soon as a team reaches or exceeds 100 points.
- This mode uses pip scoring, not one point per hand won.

Example: if the losing players hold 12 and 19 pips, the winning team receives
31 points, regardless of its own remaining tiles.

## Timing

- Each player starts every hand with 60 seconds of personal time.
- A player's clock runs during their turn.
- Normal tile plays and legal passes use a ten-second increment.
- The agreed normal-turn example is that a player starting with 60 seconds and
  completing a turn in three seconds has 67 seconds afterward.
- Once per turn, the active player may explicitly request thinking time.
- A thinking request pauses the player's clock for 20 seconds. If the turn is
  still active afterward, the clock resumes.
- A turn with a thinking request receives no increment, whether completed by a
  tile play or a legal pass.
- A thinking request pauses the clock; it does not add 20 seconds to the player's
  stored balance.
- A timeout loses the hand for the player's team, with scoring as above.

The increment's grant timing is not yet settled. Crediting it at turn start and
crediting it after a completed action can produce different timeout outcomes
when a player begins a turn with little time. The three-second example alone
does not settle that distinction.

## Details still requiring agreement

1. Specify any special placement behavior for doubles beyond ordinary matching.
2. Choose the dealing sequence and the direction of seating order.
3. Specify when the ten-second increment is granted and how a thinking request
   cancels it if already granted.
4. Specify whether opening selection and the opening move are timed, including
   whether either partner may delay the opening decision indefinitely.
5. Define the result when an action and timeout occur at the same time, and
   whether a player may finish a turn during the thinking-time pause.

## Review examples

These cases should guide later tests once their relevant rules are complete:

- Dealing gives ten tiles to each player and leaves fifteen unused, with every
  distinct tile present exactly once across all locations.
- Domination awards only the opponents' remaining pips.
- A blocked hand with totals `A1=8, A2=30, B1=12, B2=13` is won by team A,
  despite team A having the larger combined total.
- A blocked hand with totals `A1=8, A2=8, B1=12, B2=13` is won by team A.
- A blocked hand with totals `A1=8, A2=30, B1=8, B2=13` is null, awards zero,
  and retains opening for the previous opening team.
- A team on 90 points receiving 15 points wins the match with 105 points.
- A player unable to move stays on the clock until passing or hand completion.
- A legal pass receives normal increment treatment unless thinking time was
  requested during that turn.
- A thinking request pauses the clock for 20 seconds and prevents the turn's
  increment.
- A timeout awards the opponents the losing team's remaining pip total.

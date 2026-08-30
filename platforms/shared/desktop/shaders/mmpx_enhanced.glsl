// MMPX Enhanced
// Original MMPX by Morgan McGuire and Mara Gagiu, MIT license.
// Adapted for Slang by hunterk. Enhanced by CrashGG.
// Ported from libretro/slang-shaders mmpx-ex.slang to Gearboy GLSL preset uniforms.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;

uint pack_color(vec3 color)
{
    uvec3 value = uvec3(clamp(color, vec3(0.0), vec3(1.0)) * 255.0 + vec3(0.5));
    return value.r | (value.g << 8u) | (value.b << 16u) | (255u << 24u);
}

vec3 unpack_color(uint value)
{
    return vec3(float(value & 255u), float((value >> 8u) & 255u), float((value >> 16u) & 255u)) / 255.0;
}

float flag(bool value)
{
	return value ? 1.0 : 0.0;
}


float luma(vec3 col){
   //Use BT.601 standard from the CRT era
   return dot(col, vec3(0.299, 0.587, 0.114));
}

/* Constant definitions:
0.145898	:	Two short golden ratios of 1.0
0.0638587	:	Squared short golden ratio (2x) for RGB Euclidean distance
0.4377		:	Squared short golden ratio (1x) for RGB Euclidean distance
0.75			:	Squared half of RGB Euclidean distance
*/

bool simb(vec3 col1, vec3 col2) {

	vec3 diff = col1 - col2;

	float maxdiff = max(diff.r, max(diff.g, diff.b));
	float mindiff = min(diff.r, min(diff.g, diff.b));

	// Luminance weight floor: both colors must be > 0.078.
	float weight = step(0.234, min(col1.r+col1.g+col1.b, col2.r+col2.g+col2.b));

	// Find the most opposite channel: take the smallest absolute value if one is positive and the other negative; 0 if same sign
	// Filter same-sign cases using max(0.0, ...)
	// Skip team_rebel if either pixel's luminance < 0.078
	float team_rebel = min(max(0.0, maxdiff), max(0.0, -mindiff)) * weight;
	float finaldist = (maxdiff - mindiff) + team_rebel;

	float dot_diff = dot(diff, diff);

	// Equivalent to squared final distance scaled by the short golden ratio.
	float factor = (finaldist * finaldist) * 46.9787;

	return dot_diff < mix(0.0638587, 0.0, factor);
}

bool sim(vec3 col1, vec3 col2) {

	vec3 diff = col1 - col2;

	// RGB color difference range (max_diff - min_diff)
	float delta_range = max(diff.r, max(diff.g, diff.b)) - min(diff.r, min(diff.g, diff.b));

	float dot_diff = dot(diff, diff);

	// Equivalent to squared delta range scaled by the short golden ratio.
	float factor = (delta_range * delta_range) * 6.8541;

	return dot_diff < mix(0.0638587, 0.0, factor);
}

bool vi_sim(vec3 col1, uint uC1, uint uC2) {
    if (uC1==uC2) return true;
    vec3 col2 = unpack_color(uC2);
    return sim(col1, col2);
}

float mixGate(vec3 col1, vec3 col2) {

	vec3 diff = col1 - col2;

	// RGB color difference range (max_diff - min_diff)
	float delta_range = max(diff.r, max(diff.g, diff.b)) - min(diff.r, min(diff.g, diff.b));

	float dot_diff = dot(diff, diff);

	// Equivalent to squared delta range scaled by the long golden ratio.
	float factor = (delta_range * delta_range) * 2.618034;

	return step(dot_diff, mix(0.75, 0.0, factor));
}


#define eq(a,b) (a==b)

#define neq(a,b) (a!=b)

#define all_eq2(a, b1, b2) \
	( eq(a,b1) && eq(a,b2))

#define all_eq3(a, b1, b2, b3) \
	( eq(a,b1) && eq(a,b2) && eq(a,b3))

#define all_eq4(a, b1, b2, b3, b4) \
	( eq(a,b1) && eq(a,b2) && eq(a,b3) && eq(a,b4))

#define any_eq2(a, b1, b2) (eq(a,b1)||eq(a,b2))
#define any_eq3(a, b1, b2, b3) (eq(a,b1)||eq(a,b2)||eq(a,b3))
// Better than a!=b1 && a!=b2
#define none_eq2(a, b1, b2) !any_eq2(a, b1, b2)


// Allow total int2 difference across 3 channels
//#define vec_neq(a,b) (dot(abs(a-b), vec3(1.0)) > 0.01)
// Allow int2 difference per channel for 3 channels
#define vec_eq(a,b) all(lessThan(abs(a-b), vec3(0.01)))
#define vec_neq(a, b) any(greaterThan(abs(a-b), vec3(0.01)))


// Pre-define
//const vec3 testcolor  = vec3(1.0, 0.0, 1.0);  // Magenta
//const vec3 testcolor2 = vec3(0.0, 1.0, 1.0);  // Cyan
//const vec3 testcolor3 = vec3(1.0, 1.0, 0.0);  // Yellow
//const vec3 testcolor4 = vec3(1.0, 1.0, 1.0);  // White
const vec3 slopOFF   = vec3(2.0);
const vec3 slopeBAD  = vec3(4.0);
const vec3 theEXIT   = vec3(8.0);

#define mixXE mix(vX,vE,mixFactor)
#define mixXEoff mixXE+slopOFF
#define Xoff vX+slopOFF
//#define checkblack(col) ((col).g < 0.078 && (col).r < 0.1 && (col).b < 0.1)
#define checkblack(col) all(lessThan((col).rgb, vec3(0.1, 0.078, 0.1)))
#define checkwhite(col) all(greaterThan((col).rgb, vec3(0.92, 0.92, 0.92)))

//pin zz
// "Concave + Cross" weak blend (weak/no blending)
vec3 admixC(vec3 vX, vec3 vE) {
	// Weak blending. Mix if 0.618, else 1.0
	float mixFactor = mixGate(vX, vE) * (-0.381966) + 1.0;

	return mixXE;
}

// K-type forced weak blend
vec3 admixK(vec3 vX, vec3 vE) {
    vec3 diff = vX - vE;
	// mixFactor scales from 0.5-1.0 with quadratic falloff (steeper near 1.0) based on squared distance
	float mixFactor = dot(diff, diff) * 0.16666 + 0.5;
	// mixFactor scales linearly from 0.5-1.0 based on Euclidean distance
	//float mixFactor = distance(vX, vE) * 0.28867 + 0.5;
	return mixXE;
}

// L-type 2:1 slope - main corner extension
// Practical rule: this requires 4 pixels on the strict slope to be identical. Artifacts occur otherwise!
vec3 admixL(vec3 vX, vec3 vE, vec3 vS) {

    // If target X differs from reference S(sample), it's already blended once - copy directly without re-blending
	if (vec_neq(vX, vS)) return vX;

	float mixFactor = 0.381966 * mixGate(vX,vE);

    return mixXE;
}

/**************************************************************************************************************************************
 * 												Main slope + X cross-processing mechanism						                *
 ******************************************************************************************************************************** zz  */
vec3 admixX( uint A, uint B, uint C, uint D, uint E, uint F, uint G, uint H, uint I
		  , uint P, uint PA, uint PC, uint Q, uint QA, uint QG, uint R, uint RC, uint RI, uint S, uint SG, uint SI, uint AA, uint CC, uint GG
		  , float El, float Bl, float Dl, float Fl, float Hl
		  , vec3 vE, vec3 vB, vec3 vD, vec3 vC, vec3 vG
		  ) {


	bool eq_B_C = eq(B,C);
	bool eq_D_G = eq(D,G);

    // Exit if enclosed by straight walls on both sides
    if (eq_B_C && eq_D_G) return slopeBAD;


	//Pre-declare
	bool eq_B_P;		bool eq_B_PA;	bool eq_B_PC;
	bool eq_D_Q;		bool eq_D_QA;	bool eq_D_QG;
	bool eq_E_F;		bool eq_E_H;		bool eq_A_AA;

	vec3 vX;
	float mixFactor;

	bool eq_E_C = eq(E,C);
	bool eq_E_G = eq(E,G);
    bool eq_A_P = eq(A,P);
    bool eq_A_Q = eq(A,Q);
    bool comboE3 = eq_E_C && eq_E_G;
    bool comboA3 = eq_A_P && eq_A_Q;

/*=========================================
                    B != D
  ==================================== zz */
if (neq(B,D)){

	// E == A violates logic, exit
	if (eq(E,A)) return slopeBAD;

	// B-D unconnected? Removed

	// If B and D differ significantly (more than either to center E), exit
	float diffBD = abs(Bl-Dl);
	if (diffBD > El-Bl || diffBD > El-Dl) return slopeBAD;


	// X is blend of B and D
	vX = mix(vB, vD, 0.5);

	mixFactor = 0.381966 * mixGate(vX,vE);

	eq_B_PC = eq(B,PC);
	eq_D_QG = eq(D,QG);
 
	// Strong pattern set
    if (none_eq2(A,B,D)){
		if (comboA3) return mixXEoff;
		if ( eq_A_P && eq_B_PC && !eq_B_C ) return mixXEoff;
		if ( eq_A_Q && eq_D_QG && !eq_D_G ) return mixXEoff;

		// Double slope clamping BD (direction-aware)
		if ( eq_A_P && eq_E_G ) return mixXEoff;
		if ( eq_A_Q && eq_E_C ) return mixXEoff;

		// Hollow L inner corner
		if ( eq_E_C && eq_D_G ) return mixXEoff;
		if ( eq_E_G && eq_B_C ) return mixXEoff;

}
    // E-side triple match
    if ( comboE3 ) return mixXEoff;

	// Original rule with added slope condition
    if ( eq_E_C && eq_B_PC && neq(B,P)) return mixXEoff;
	if ( eq_E_G && eq_D_QG && neq(D,Q)) return mixXEoff;

	eq_E_F = eq(E,F);

	// F == H
	if (eq(F,H)) {

		// Double slope (exclude single-pixel C enclosure, BD are weakly connected)
		if ( eq_E_C && !eq_D_G && (!eq_E_F||neq(E,P)) ) return mixXEoff;
		if ( eq_E_G && !eq_B_C && (!eq_E_F||neq(E,Q)) ) return mixXEoff;

		// F+H extension
		if ( !eq_E_F && eq_B_PC && eq(F,RC) ) return mixXEoff;
		if ( !eq_E_F && eq_D_QG && eq(H,SG) ) return mixXEoff;
	}

    return slopeBAD;
} // B != D


						/*********  B == D  *********/
  
	// Prevent font edges from being crushed by black background on 3 sides
	bool Xisblack = checkblack(vB);
	if ( Xisblack && El >0.5 && (Fl<0.078 || Hl<0.078) ) return theEXIT;

	vX = vB;

	mixFactor = 0.381966 * mixGate(vX,vE);

	bool B_slope;	bool B_tower;	bool B_wall;
    bool D_slope;	bool D_tower;	bool D_wall;
	bool En3;
    #define En4square En3&&eq(E,I)

/*===================================================
                    E - A intersection
  ============================================== zz */
if (eq(E,A)) {

    // Special pattern: dithering
    // Goal: force blending

	eq_E_F = eq(E,F);
	eq_E_H = eq(E,H);

	bool Eisblack = checkblack(vE);

	// 1. Dither center
    if ( comboE3 && !eq_E_F && !eq_E_H && eq(E,I) ) {

		// Exit if center E is black (KOF '96 power gauge, Punisher belt) - avoid high-contrast blending
		if (Eisblack) return theEXIT;
		// Practical 1: skip black B points (handled by normal logic)
		// Mix if 0.381966, else 0.618034
		mixFactor = 0.618034 * (1.0 - mixFactor);
		return mixXEoff;
	}

	eq_A_AA = eq(A,AA);

	// 2. Dither edge
    if ( comboA3 && eq_A_AA && none_eq2(A,PA,QA) )  {	
		if (Eisblack) return theEXIT;
		// Mix if 0.381966, else 0.618034
		mixFactor = 0.618034 * (1.0 - mixFactor);
        // Strong blend for layered gradient edges
		if ( neq(B,PA) && eq(PA,QA) ) return mixXEoff;
        // Remainder: perfect cross = dither edge, use weak blend
		// Practical: no special handling needed for health bar borders
        // Floor weak blend. Mix if 0.618, else 0.854
		// Note: mixFactor already modified above
		mixFactor += 0.236068;
		return mixXEoff;
	}

    eq_B_PC = eq(B,PC);
    eq_B_PA = eq(B,PA);
    eq_D_QG = eq(D,QG);
    eq_D_QA = eq(D,QA);

	// Subsequent checks skip Eisblack
	// 3. Half-dither (usually silhouette edge shading), use weak blend

	if ( comboE3 && comboA3 &&
		(eq_B_PC || eq_D_QG) && eq_D_QA && eq_B_PA) {
        // Floor weak blend. Mix if 0.618, else 0.854
		mixFactor = mixFactor * (-0.618034) + 0.8541;
        return mixXEoff;
	}

    // 4. Quarter-dither (prevents ugly "pinky" artifacts: SF2 Guile plane, Cadillacs and Dinosaurs character select)

	if ( comboE3 && eq_A_P
		 && eq_B_PA && eq_D_QA && eq_D_QG
		 && eq_E_H
		) {// Floor weak blend. Mix if 0.618, else 0.854
		mixFactor = mixFactor * (-0.618034) + 0.8541;
        return mixXEoff;
		}

	if ( comboE3 && eq_A_Q
		 && eq_B_PA && eq_D_QA && eq_B_PC
		 && eq_E_F
		) {// Floor weak blend. Mix if 0.618, else 0.854
		mixFactor = mixFactor * (-0.618034) + 0.8541;
        return mixXEoff;
		}


    // A-side triple match (strong pattern, after dither logic)
	if (comboA3) return Xoff;

    // E-side triple match (after comboA3)
    if (comboE3) return mixXEoff;

	eq_B_P = eq(B, P);
	eq_D_Q = eq(D, Q);

	B_slope = eq_B_PC && !eq_B_P && !eq_B_C && !eq_B_PA;
	D_slope = eq_D_QG && !eq_D_Q && !eq_D_G && !eq_D_QA;

	B_wall = eq_B_C && !eq_B_PC && !eq_B_P;	// Removed one misalignment check
	D_wall = eq_D_G && !eq_D_QG && !eq_D_Q;	// Removed one misalignment check
	
	B_tower = eq_B_P && !eq_B_PC && !eq_B_C && !eq_B_PA;
	D_tower = eq_D_Q && !eq_D_QG && !eq_D_G && !eq_D_QA;


	if ( B_slope && eq_E_G ) return mixXEoff;
	if ( D_slope && eq_E_C ) return mixXEoff;


// E/B/D zone scoring system

    float scoreE = 0.0; float scoreB = 0.0; float scoreD = 0.0; float scoreZ = 0.0;

//	E Zone
    if (eq_E_C) {
		scoreE += 1.0 + flag(eq(F,H)) + flag(B_slope);
		scoreE -= flag(all_eq2(E,P,PC)&&!D_wall);
	}

    if (eq_E_G) {
		scoreE += 1.0 + flag(eq(F,H)) + flag(D_slope);
		scoreE -= flag(all_eq2(E,Q,QG)&&!B_wall);
    }

	// Higher priority than rectangles
	scoreE += flag(B_slope && eq_A_Q || D_slope && eq_A_P);

    En3 = eq_E_F && eq_E_H;

	// Clear 4/6-square: early exit, skip final Z long-slope check
	if ( scoreE<0.1 && mixFactor<0.1 && En4square && eq(E,S)==eq(E,SI) && eq(E,R)==eq(E,RI) ) return theEXIT;

	// No score for En3
	//if ( scoreE==0 && En3 ) scoreE += 1;

	// Single bar
	if ( scoreE<0.1 && !En3 && neq(E,I) ) {
		if ( B_wall && eq_E_F ) return theEXIT;
		if ( D_wall && eq_E_H ) return theEXIT;
    }

	// Lower priority than single bar
	scoreE += flag(B_slope && eq_A_P || D_slope && eq_A_Q);

    if ( !En3 && eq(F,H) ) {
		if (Eisblack) return slopeBAD;		// Black single pixel
		// slope+eq_F_H combination disabled (avoids bubbles with inner L in BD zone)
		//scoreE += flag(B_slope&&neq(C,F))+flag(D_slope&&neq(G,H));

		bool condZ1 = B_wall && (eq(F,R) || eq(F,RC) || eq(G,H) || eq(F,I));
		bool condZ2 = D_wall && (eq(C,F) || eq(H,SG) || eq(H,S) || eq(F,I));
		scoreZ = flag(condZ1 || condZ2);
    }


//	B Zone

    if (eq_B_PA) {
		scoreB -= 1.0 + flag(eq(P,C)) + flag(eq_A_AA);
	}

	if (eq(P,C)){
		scoreB -= flag(eq_A_AA); 
		// Critical: prevent Z scoring on mirrored shapes when only F==H
		// Equivalent: if (scoreE==0) scoreZ = 0;
		scoreZ *= flag(scoreE < 0.1);
	}

//  D Zone

    if (eq_D_QA) {
		scoreD -= 1.0 + flag(eq(G,Q)) + flag(eq_A_AA);
	}

	if (eq(G,Q)){
		scoreD -= flag(eq_A_AA);
		// Same logic as B zone
		scoreZ *= flag(scoreE < 0.1);
	}

    float scoreFinal = scoreE + scoreB + scoreD + scoreZ ;

	// Long slope: return unblended vX if B/D zones have no penalties and form a gentle slope
	scoreFinal += flag(min(scoreB,scoreD) > -0.1 && (B_wall && D_tower || B_tower && D_wall)) * 2.0;

	// Set mixFactor to 0 (no blend) if scoreFinal >= 2, return vX
	mixFactor *= (1.0 - step(1.9, scoreFinal));
	// Return mixXE if scoreFinal >=1, else slopeBAD
	return mixXE + slopeBAD*(1.0 - step(0.9, scoreFinal));

}	// E == A

	
/*===============================================
                 Main rule: E - C - G
  ========================================== zz */

    if (eq_E_C ) {
		if (comboA3) return vX;
		if (comboE3) return mixXE;
		if (all_eq2(B,A,PA) && all_eq3(E,F,P,PC)) return theEXIT;
		return mixXE;
	}

	if (eq_E_G) {
		if (comboA3) return vX;
		if (comboE3) return mixXE;
		if (all_eq2(D,A,QA) && all_eq3(E,H,Q,QG)) return theEXIT;
		return mixXE;
	}


/*=========================================================
                   F - H / B+ D+ extension rules
  ==================================================== zz */

	// This section handles leftovers from previous filters; center En4square is naturally walled off from BD logic
    // B-D unconnected? No longer needed with new "double slope" logic
	// Rule 1: Flatten inner L corner, not outer
	// Rule 2: Flatten outer "" edge, not inner

    bool eq_A_B = eq(A,B);
    bool eq_F_H = eq(F,H);

	eq_B_P  = eq(B,P);
	eq_B_PC = eq(B,PC);
	eq_B_PA = eq(B,PA);
	eq_D_Q  = eq(D,Q);
	eq_D_QG = eq(D,QG);
	eq_D_QA = eq(D,QA);

	B_slope = eq_B_PC && !eq_B_P && !eq_B_C;
	D_slope = eq_D_QG && !eq_D_Q && !eq_D_G;
	B_tower = eq_B_P && !eq_B_PC && !eq_B_C && !eq_B_PA;
	D_tower = eq_D_Q && !eq_D_QG && !eq_D_G && !eq_D_QA;
	B_wall = eq_B_C && !eq_B_PC && !eq_B_P;
	D_wall = eq_D_G && !eq_D_QG && !eq_D_Q;


//	1. B-D hollow slope
    if (!eq_A_B) {

        // A-side triple match (high priority)
		// Note: comboA3 only valid when A!=B in this section
        if (comboA3) return Xoff;

        if ( (B_slope||B_tower) && (D_slope||D_tower) ) return Xoff;

        if ( B_slope && eq_A_P ) return mixXEoff;
        if ( D_slope && eq_A_Q ) return mixXEoff;

        if ( (B_slope || D_slope) && eq_F_H ) return mixXEoff;

        if ( B_slope && eq(H,SG) ) return mixXEoff;
        if ( D_slope && eq(F,RC) ) return mixXEoff;

        if ( B_slope && eq_A_Q && eq(Q,QG) ) return mixXEoff;
        if ( D_slope && eq_A_P && eq(P,PC) ) return mixXEoff;

    }



	bool sim_EC = sim(vE, vC);
	bool sim_EG = sim(vE, vG);

	// Exit if center E is a high-contrast single pixel
	// Tighten threshold for bright E
	float E_lumDiff = mix(0.381966, 0.145898, max((El - 0.8541),0.0) * 6.8541);

	// High contrast to neighbors (lower priority than slope detection)
    if ( mixFactor<0.1 && !sim_EC && !sim_EG && neq(E,I) && abs(El-Fl)>E_lumDiff && abs(El-Hl)>E_lumDiff ) return slopeBAD;


	eq_E_F = eq(E,F);
	eq_E_H = eq(E,H);

    // Long gentle slope (preserve squares for later En4square check)
	if ( eq_B_C && eq_D_Q ) {
		if ( eq(P,PC) && eq(A,QA) && !eq_D_QG && eq_E_F && !eq_E_H && eq(H,I)) return theEXIT;
		if ( eq_A_B ) return slopeBAD;
		if ( B_wall && D_tower && eq_E_F) return vX;
		return mixXEoff;
	}

	if ( eq(D,G) && eq(B,P)) {
		if ( eq(Q,QG) && eq(A,PA) && !eq_B_PC && eq_E_H && !eq_E_F && eq(F,I)) return theEXIT;
		if ( eq_A_B ) return slopeBAD;
		if ( B_tower && D_wall && eq_E_H) return vX;
		return mixXEoff;
	}


    En3 = eq_E_F && eq_E_H;

    // Wall-enclosed 4-square (En3 && eq(E,I))
	if ( En4square ) {  // Must come after previous rule
        // Exit for solid L enclosure (font edges, building corners)
        // Solid L corner / high-contrast clear 4-square / 6-rectangle (no need for eq(G,H)/eq(C,F) checks)
        if ( ( eq_B_C || eq_D_G) && eq_A_B) return theEXIT;
        if ( ( eq_B_C || eq_D_G || mixFactor<0.1) && (eq(E,S) == eq(E, SI) && eq(E,R) == eq(E, RI)) ) return theEXIT;
        return mixXEoff;
    }

	// BD non-wall solid shapes
	if (!eq_B_C && !eq_D_G ) {
		 // B/D semi-solid 1 (requires F-H)
		if ( comboA3 && eq_F_H ) return Xoff;

		// B/D semi-solid 2 (with "definite rounding" trend)
		if ( comboA3&&eq_B_PC&&eq(C,CC) ) return Xoff;
		if ( comboA3&&eq_D_QG&&eq(G,GG) ) return Xoff;

		// Exit if B/D unconnected and not En3 (required for this branch)
		if ( !eq_B_P && !eq_B_PC && !eq_D_Q && !eq_D_QG && !En3 ) return slopeBAD;

		// 3 diagonal gradients (after above filter)
		if (eq_A_Q&&sim_EC) return mixXEoff;
		if (eq_A_P&&sim_EG) return mixXEoff;
		if (sim_EC&&sim_EG ) return mixXEoff;
	}
	
    // Wall-enclosed triangle (remove solid corner, pass to next rule)
 	if ( En3 && eq_A_B) return theEXIT;

    // F - H
	// Rule: connect inner L corner, not outer
	if (eq_F_H) {

		// F-H triple pattern (huge quality boost! priority over A==B)
		if ( eq_B_PC&&eq(F,RC) || eq_D_QG&&eq(H,SG) ) return mixXEoff;

		if (eq_A_B) return slopeBAD;

		if ( eq_B_C || eq_D_G) return mixXEoff;
		if ( eq_B_PC || eq_D_QG) return mixXEoff;

	}

	return slopeBAD;

}	// admixX


vec3 admixS( uint A, uint B, uint C, uint D, uint E, uint F, uint G, uint H, uint I
		   , uint R, uint RC, uint RI, uint S, uint SG, uint SI, uint II, uint CC
		   , vec3 vE, vec3 vF, vec3 vC
		   ) {

			//                                    B  .
			//                                           Zone 4
			//                                     
			//                                      S


    if (any_eq2(F,C,I)) return vE;

	// Skip destructive patterns on opposite side
	if ( (eq(F,RI) || eq(G,S) || eq(R, RI)) && neq(R,I) ) return vE;

    if (eq(H, S) && none_eq2(H,I,SG)) return vE;

    if ( eq(R, RC) || eq(G,SG) ) return vE;

	// Extend one extra pixel in trend direction if E is white in D==E==C pattern (Street Fighter II Guile's face)
	if ( checkwhite(vE) && all_eq2(E,C,D) && none_eq2(E,RC,CC)) return vE;

	// Old opposite trend check
	// if ( none_eq2(I,H,S) && (neq(SI,RI) || eq(I,II)) ) return vE;


	#define vX vF
	float mixFactor = 0.381966 * mixGate(vX,vE);

	if ( eq(E,C) && (eq(E,D)||eq(B,D)) ) return mixXE;

	bool sim_E_C = sim(vE,vC);

	if ( sim_E_C && eq(E,D) && eq(B,C) ) return mixXE;

	if ( (sim_E_C || mixFactor>0.1) && all_eq2(B,C,D) ) return mixXE;

    return vE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////// zz

void main()
{
#define srcf(c,d) texture(Source, vTexCoord + vec2(c,d) * SourceSize.zw).rgb
#define src(c,d) pack_color(srcf(c,d))

	vec3 vE = srcf(0.0, 0.0);
	vec3 vB = srcf(0.0, -1.0);
	vec3 vD = srcf(-1.0, 0.0);
	vec3 vF = srcf(+1.0, 0.0);
	vec3 vH = srcf(0.0, +1.0);

    // Initialize for fast return
    FragColor = vec4(vE, 1.0);

    uint E = pack_color(vE);
    uint B = pack_color(vB);
    uint D = pack_color(vD);
    uint F = pack_color(vF);
    uint H = pack_color(vH);


    bool eq_E_D = eq(E,D);
    bool eq_E_F = eq(E,F);
    bool eq_E_B = eq(E,B);
    bool eq_E_H = eq(E,H);
    bool eq_B_H = eq(B,H);
    bool eq_D_F = eq(D,F);

// Skip horizontal/vertical 3x1 lines
bool skiprest = (eq_E_D && eq_E_F) || (eq_E_B && eq_E_H) || (eq_B_H && eq_D_F);
if (!skiprest) {



    //Sample 5x5 grid
	vec3 vA = srcf(-1.0, -1.0);
	vec3 vC = srcf(+1.0, -1.0);
	vec3 vG = srcf(-1.0, +1.0);
	vec3 vI = srcf(+1.0, +1.0);

    uint A = pack_color(vA);
    uint C = pack_color(vC);
    uint G = pack_color(vG);
    uint I = pack_color(vI);

	uint P  = src( 0.0, -2.0);
	uint Q  = src(-2.0,  0.0);
	uint R  = src(+2.0,  0.0);
	uint S  = src( 0.0, +2.0);

	uint PA = src(-1.0, -2.0);
	uint PC = src(+1.0, -2.0);
	uint QA = src(-2.0, -1.0);
	uint QG = src(-2.0, +1.0); //             AA    PA    [P]   PC    CC
	uint RC = src(+2.0, -1.0); //                
	uint RI = src(+2.0, +1.0); //             QA   A   B  C   RC
	uint SG = src(-1.0, +2.0); //                
	uint SI = src(+1.0, +2.0); //            [Q]   D   E  F   [R]
	uint AA = src(-2.0, -2.0); //                
	uint CC = src(+2.0, -2.0); //             QG   G   H  I   RI
	uint GG = src(-2.0, +2.0); //                
	uint II = src(+2.0, +2.0); //             GG    SG    [S]   SI    II

    // Default: nearest-neighbor upscale
    vec3 J = vE;    vec3 K = vE;    vec3 L = vE;    vec3 M = vE;

    // ------------------------ Boundary check --- Libretro -----------------------
    // Define dummy pixel color (impossible value)
    #define fakeColor vec3(1.234)
	// 1. Precompute integer pixel coordinates.
    vec2 pixelPos = vTexCoord * SourceSize.xy;
    ivec2 currPixel = ivec2(floor(pixelPos)); // Current pixel integer coords (0-based)
    ivec2 texSize = ivec2(SourceSize.xy); // Actual texture pixel dimensions

    // 3. Check edge overflow - use dummy color to influence luminance logic
    if (currPixel.y - 1 < 0) vB = fakeColor;          // Top pixel overflow (y-1 < 0)
    if (currPixel.x - 1 < 0) vD = fakeColor;          // Left pixel overflow (x-1 < 0)
    if (currPixel.x + 1 >= texSize.x) vF = fakeColor; // Right pixel overflow (x+1 >= width)
    if (currPixel.y + 1 >= texSize.y) vH = fakeColor; // Bottom pixel overflow (y+1 >= height)
    // ----------------------------------------------------------------------

// Precompute luminance
    float Bl = luma(vB);
    float Dl = luma(vD);
    float El = luma(vE);
    float Fl = luma(vF);
    float Hl = luma(vH);


// 	Pre-calcs
    bool eq_B_D = eq(B,D);
    bool eq_B_F = eq(B,F);
    bool eq_D_H = eq(D,H);
    bool eq_F_H = eq(F,H);

    // Any opposite pair encloses center
    bool oppoPix =  eq_B_H || eq_D_F;
	// Flag if caught by 1:1 slope rule and entered admixX
    bool slope1 = false;    bool slope2 = false;    bool slope3 = false;    bool slope4 = false;
	// Standard pixel returned successfully from 1:1 slope rule
    bool slope1ok = false;  bool slope2ok = false;  bool slope3ok = false;  bool slope4ok = false;
    bool slope1end = false;  bool slope2end = false;  bool slope3end = false;  bool slope4end = false;
	// slopeBAD: entered admixX but returned E (for at least one JKLM)
    // slopOFF: returned with OFF flag - skip long slope calculations


// B - D
	if (
		(!eq_E_B && !eq_E_D && !oppoPix) && (!eq_D_H && !eq_B_F)
	 && (eq(E,A) || El>=Dl&&El>=Bl) && ( (El<Dl&&El<Bl) || none_eq2(A,B,D) || neq(E,P) || neq(E,Q) )
	 && ( eq_B_D &&(eq(E,A)||eq(B,PC)||eq(D,QG)||sim(vE,vC)||sim(vE,vG)) || simb(vB,vD)&&(eq_F_H||eq(E,C)||eq(E,G)) )
		) {
		J=admixX(A,B,C,D,E,F,G,H,I
				,P,PA,PC,Q,QA,QG,R,RC,RI,S,SG,SI,AA,CC,GG
				,El, Bl, Dl, Fl, Hl
				,vE, vB, vD, vC, vG
				);
		slope1 = true;			// Mark on entry
		slope1ok = (J.b < 1.1);	// Valid pixel
		slope1end = (J.b < 3.1);	// Modified, skip long slope (mostly simBD)
		skiprest = (J.b > 7.1);	// theEXIT
		J = (J.b > 3.1) ? vE :		// Restore vE for slopeBAD/theEXIT
			(J.b > 1.1) ? (J - 2.0) :// slopeoff
			J;					// Normal pixel [0-1.0]
	}
// B - F
	if ( !slope1
	 && (!eq_E_B && !eq_E_F && !oppoPix) && (!eq_B_D && !eq_F_H)
	 && (eq(E,C) || El>=Bl&&El>=Fl) && ( (El<Bl&&El<Fl) || none_eq2(C,B,F) || neq(E,P) || neq(E,R) )
	 && ( eq_B_F &&(eq(E,C)||eq(B,PA)||eq(F,RI)||sim(vE,vA)||sim(vE,vI)) || simb(vB,vF)&&(eq_D_H||eq(E,A)||eq(E,I)) ) 
	 ) {
		K=admixX(C,F,I,B,E,H,A,D,G
				,R,RC,RI,P,PC,PA,S,SI,SG,Q,QA,QG,CC,II,AA
				,El,Fl,Bl,Hl,Dl
				,vE,vF,vB,vI,vA
				);
		slope2 = true;
		slope2ok = (K.b < 1.1);
		slope2end = (K.b < 3.1);
		skiprest = (K.b > 7.1);
		K = (K.b > 3.1) ? vE :	
			(K.b > 1.1) ? (K - 2.0) :
			K;
	}
// D - H
	if ( !slope1 && !skiprest
	 && (!eq_E_D && !eq_E_H && !oppoPix) && (!eq_F_H && !eq_B_D)
	 && (eq(E,G) || El>=Hl&&El>=Dl)  &&  ((El<Hl&&El<Dl) || none_eq2(G,D,H) || neq(E,S) || neq(E,Q))
	 &&	( eq_D_H &&(eq(E,G)||eq(D,QA)||eq(H,SI)||sim(vE,vA)||sim(vE,vI)) || simb(vD,vH)&&(eq_B_F||eq(E,A)||eq(E,I)) )
	 ) {
		L=admixX(G,D,A,H,E,B,I,F,C
				,Q,QG,QA,S,SG,SI,P,PA,PC,R,RI,RC,GG,AA,II
				,El,Dl,Hl,Bl,Fl
				,vE,vD,vH,vA,vI
				);
		slope3 = true;
		slope3ok = (L.b < 1.1);
		slope3end = (L.b < 3.1);
		skiprest = (L.b > 7.1);
		L = (L.b > 3.1) ? vE :	
			(L.b > 1.1) ? (L - 2.0) :
			L;
	}
// F - H
	if ( !slope2 && !slope3 && !skiprest
	 && (!eq_E_F && !eq_E_H && !oppoPix) && (!eq_B_F && !eq_D_H)
	 && (eq(E,I) || El>=Fl&&El>=Hl)  &&  ((El<Fl&&El<Hl) || none_eq2(I,F,H) || neq(E,R) || neq(E,S))
	 && ( eq_F_H &&(eq(E,I)||eq(F,RC)||eq(H,SG)||sim(vE,vC)||sim(vE,vG)) || simb(vF,vH)&&(eq_B_D||eq(E,C)||eq(E,G)) )
	  ) {
		M=admixX(I,H,G,F,E,D,C,B,A
				,S,SI,SG,R,RI,RC,Q,QG,QA,P,PC,PA,II,GG,CC
				,El,Hl,Fl,Dl,Bl
				,vE,vH,vF,vG,vC
				);
		slope4 = true;
		slope4ok = (M.b < 1.1);
		slope4end = (M.b < 3.1);
		skiprest = (M.b > 7.1);
		M = (M.b > 3.1) ? vE :	
			(M.b > 1.1) ? (M - 2.0) :
			M;
	}


//  Long gentle 2:1 slope  (P100)

	if (slope4ok) { //zone4 long slope
		// Original ext 1: pass neighbor to admixL to prevent double-blending
		// Original ext 2: prevent L-shape reoccurrence within opposite pixels unless walled
		if (all_eq2(R,F,G) && neq(R, RC) && (neq(Q,G)||eq(Q, QA))) {L=admixL(M,L,vH); skiprest = true;}
		// vertical
		if (all_eq2(S,H,C) && neq(S, SG) && (neq(P,C)||eq(P, PA))) {K=admixL(M,K,vF); skiprest = true;}
	}

	if (slope3ok) { //zone3 long slope
		// horizontal
		if (all_eq2(Q,D,I) && neq(Q, QA) && (neq(R,I)||eq(R, RC))) {M=admixL(L,M,vH); skiprest = true;}
		// vertical
		if (all_eq2(S,H,A) && neq(S, SI) && (neq(A,P)||eq(P, PC))) {J=admixL(L,J,vD); skiprest = true;}
	}

	if (slope2ok) { //zone2 long slope
		// horizontal
		if (all_eq2(R,F,A) && neq(R, RI) && (neq(A,Q)||eq(Q, QG))) {J=admixL(K,J,vB); skiprest = true;}
		// vertical
		if (all_eq2(P,B,I) && neq(P, PA) && (neq(I,S)||eq(S, SG))) {M=admixL(K,M,vF); skiprest = true;}
	}

	if (slope1ok) { //zone1 long slope
		// horizontal
		if (all_eq2(Q,D,C) && neq(Q, QG) && (neq(C,R)||eq(R, RI))) {K=admixL(J,K,vB); skiprest = true;}
		// vertical
		if (all_eq2(P,B,G) && neq(P, PC) && (neq(G,S)||eq(S, SI))) {L=admixL(J,L,vD); skiprest = true;}
	}

// Longslope complete - exit early; diagonal sawslope unlikely
// Note: sawslope entry cannot exclude diagonal slopes (including slopeok)
if (!skiprest && !oppoPix) {


        // horizontal bottom
    if (!eq_E_H && none_eq2(H,A,C)) {

        //                                    A B  
        //                                  Q D          Zone 4
        //					                 I
        //					                  
        // (!slope3 && D!=H) required to fully exclude trend
        if ( (!slope2 && !eq_B_F) && (!slope3 && !eq_D_H) && (!slope4end && !eq_F_H) &&
            !eq_E_F && eq(R,H) && eq(F,G) ) {
            M = admixS( A, B, C, D, E, F, G, H, I
                      , R, RC, RI, S, SG, SI, II, CC
                      , vE, vF, vC
                      );
            skiprest = true;}

        //                                    A  C
        //                                      R       Zone 3
        //                                     G  
        //					                   
        if ( !skiprest && (!slope1 && !eq_B_D) && (!slope4 && !eq_F_H) && (!slope3end && !eq_D_H) &&
             !eq_E_D && eq(Q,H) && eq(D,I) ) {
            L = admixS( C, B, A, F, E, D, I, H, G
                      , Q, QA, QG, S, SI, SG, GG, AA
                      , vE, vD, vA
                      );
            skiprest = true;}
    }

    // horizontal up
    if ( !skiprest && !eq_E_B && none_eq2(B,G,I)) {

        //					                   
        //                                      
        //                                            Zone 2
        //                                     H  I  .
        if ( (!slope1 && !eq_B_D)  && (!slope4 && !eq_F_H) && (!slope2end && !eq_B_F) &&
              !eq_E_F && eq(B,R) && eq(A,F) ) {
            K = admixS( G, H, I, D, E, F, A, B, C
                      , R, RI, RC, P, PA, PC, CC, II
                      , vE, vF, vI
                      );
            skiprest = true;}

        //					                  
        //                                    A  
        //                                     R        Zone 1
        //                                  . G  I
        if ( !skiprest && (!slope2 && !eq_B_F) && (!slope3 && !eq_D_H) && (!slope1end && !eq_B_D) &&
             !eq_E_D && eq(B,Q) && eq(C,D) ) {
            J = admixS( I, H, G, F, E, D, C, B, A
                      , Q, QG, QA, P, PC, PA, AA, GG
                      , vE, vD, vG
                      );
            skiprest = true;}

    }

    // vertical left
    if ( !skiprest && !eq_E_D && none_eq2(D,C,I) ) {

        //                                     B 
        //                                  Q    R
        //                                      I        Zone 3
        //                                        
        if ( (!slope1 && !eq_B_D) && (!slope4 && !eq_F_H) && (!slope3end && !eq_D_H) &&
              !eq_E_H && eq(D,S) && eq(A,H) ) {
            L = admixS( C, F, I, B, E, H, A, D, G
                      , S, SI, SG, Q, QA, QG, GG, II
                      , vE, vH, vI
                      );
            skiprest = true;}

        //                                       
        //                                    A  C
        //                                  Q   F R       Zone 1
        //                                     
        if ( !skiprest && (!slope3 && !eq_D_H) && (!slope2 && !eq_B_F) && (!slope1end && !eq_B_D) &&
              !eq_E_B && eq(P,D) && eq(B,G) ) {
            J = admixS( I, F, C, H, E, B, G, D, A
                      , P, PC, PA, Q, QG, QA, AA, CC
                      , vE, vB, vC
                      );
            skiprest = true;}

    }

    // vertical right
    if ( !skiprest && !eq_E_F && none_eq2(F,A,G) ) { // right

        //                                    A B 
        //                                  Q D   R
        //                                    G  I        Zone 4
        //                                    . 
        if ( (!slope2 && !eq_B_F) && (!slope3 && !eq_D_H) && (!slope4end && !eq_F_H) &&
              !eq_E_H && eq(S,F) && eq(H,C) ) {
            M = admixS( A, D, G, B, E, H, C, F
                      , I, S, SG, SI, R, RC, RI, II, GG
                      , vE, vH, vG
                      );
            skiprest = true;}

        //                                     
        //                                    A  C
        //                                  Q D   R        Zone 2
        //                                    G H 
        if ( !skiprest && (!slope1 && !eq_B_D) && (!slope4 && !eq_F_H) && (!slope2end && !eq_B_F) &&
             !eq_E_B && eq(P,F) && eq(B,I) ) {
            K = admixS( G, D, A, H, E, B, I, F, C
                      , P, PA, PC, R, RI, RC, CC, AA
                      , vE, vB, vA
                      );
            skiprest = true;}

    } // vertical right
} // sawslope

// Exit after sawslope; old logic: skiprest||slopeBAD (still uses slopeOFF (weak) and slopeok (strong) with minor effect)
skiprest = skiprest||slope1||slope2||slope3||slope4;

/**************************************************
       "Concave + Cross" blending (P100)	   
 *************************************************/
// Use approximate pixels for cross distant edges - improves horizontal lines + aliasing and layered gradients
// Example: glowing text in SFIIIn2 intro, Japanese houses in SFZ3mix, Wolf Fang intro

vec3 vT;		// Temp T

if (!skiprest &&
    Bl<El && !eq_E_D && !eq_E_F && eq_E_H && none_eq2(E,A,C) && all_eq2(G,H,I) && vi_sim(vE,E,S) ) { // TOP

    if (eq_B_D||eq_B_F) { J=admixC(vB,J);    K=J;
        if (eq_D_F) { L=mix(J,L, 0.61804);   M=L; }
    } else { vT = El-Bl < abs(El-Dl) ? vB : vD;  J=admixC(vT,J);
            if (eq_D_F) { K=J;  L=mix(J,L, 0.61804);    M=L; }
            else {vT = El-Bl < abs(El-Fl) ? vB : vF; 		K=admixC(vT,K); }
           }

   skiprest = true;
}

if (!skiprest &&
    Hl<El && !eq_E_D && !eq_E_F && eq_E_B && none_eq2(E,G,I) && all_eq2(A,B,C) && vi_sim(vE,E,P) ) { // BOTTOM

    if (eq_D_H||eq_F_H) { L=admixC(vH,L);    M=L;
        if (eq_D_F) { J=mix(L,J, 0.61804);   K=J; }
    } else { vT = El-Hl < abs(El-Dl) ? vH : vD;  L=admixC(vT,L);
            if (eq_D_F) { M=L;  J=mix(L,J, 0.61804);    K=J; }
            else { vT = El-Hl < abs(El-Fl) ? vH : vF;    M=admixC(vT,M); }
           }

   skiprest = true;
}

if (!skiprest &&
    Fl<El && !eq_E_B && !eq_E_H && eq_E_D && none_eq2(E,C,I) && all_eq2(A,D,G) && vi_sim(vE,E,Q) ) { // RIGHT

    if (eq_B_F||eq_F_H) { K=admixC(vF,K);    M=K;
        if (eq_B_H) { J=mix(K,J, 0.61804);   L=J; }
    } else { vT = El-Fl < abs(El-Bl) ? vF : vB;  K=admixC(vT,K);
            if (eq_B_H) { M=K;  J=mix(K,J, 0.61804);    L=J; }
            else { vT = El-Fl < abs(El-Hl) ? vF : vH;    M=admixC(vT,M); }
           }

   skiprest = true;
}

if (!skiprest &&
    Dl<El && !eq_E_B && !eq_E_H && eq_E_F && none_eq2(E,A,G) && all_eq2(C,F,I) && vi_sim(vE,E,R) ) { // LEFT

    if (eq_B_D||eq_D_H) { J=admixC(vD,J);    L=J;
        if (eq_B_H) { K=mix(J,K, 0.61804);   M=K; }
    } else { vT = El-Dl < abs(El-Bl) ? vD : vB;  J=admixC(vT,J);
            if (eq_B_H) { L=J;   K=mix(J,K, 0.61804);    M=K; }
            else { vT = El-Dl < abs(El-Hl) ? vD : vH;    L=admixC(vT,L); }
           }

   skiprest = true;
}

/*
     
 
         Scorpion pattern (P99). Resembles Matrix sentinels. Smooths regular staggered pixels.
*/
// Practical rules:
// 1. Do NOT use approximate pixels (causes artifacts)
// 2. Shorten scorpion tail by 1 pixel for bright centers to catch more patterns
// Scorpion is exclusive - won't trigger if any earlier rule matched (entered)

if (!skiprest && !eq_E_F&&eq_E_D&&eq_B_F&&eq_F_H && all_eq2(E,C,I) && (eq(E,Q)||El>Fl) && neq(F,src(+3.0, 0.0)) ) {K=admixK(vF,K); M=K;skiprest=true;}	// RIGHT
if (!skiprest && !eq_E_D&&eq_E_F&&eq_B_D&&eq_D_H && all_eq2(E,A,G) && (eq(E,R)||El>Dl) && neq(D,src(-3.0, 0.0)) ) {J=admixK(vD,J); L=J;skiprest=true;}	// LEFT
if (!skiprest && !eq_E_H&&eq_E_B&&eq_D_H&&eq_F_H && all_eq2(E,G,I) && (eq(E,P)||El>Hl) && neq(H,src(0.0, +3.0)) ) {L=admixK(vH,L); M=L;skiprest=true;}	// BOTTOM
if (!skiprest && !eq_E_B&&eq_E_H&&eq_B_D&&eq_B_F && all_eq2(E,A,C) && (eq(E,S)||El>Bl) && neq(B,src(0.0, -3.0)) ) {J=admixK(vB,J); K=J;}				// TOP

	//Final write
	vec2 a = fract(pixelPos);	// pixelPos defined in edge check

	FragColor.rgb = (a.x < 0.5) ? (a.y < 0.5 ? J : L) : (a.y < 0.5 ? K : M);
    FragColor.a = 1.0;

	//float fx = step(0.5, a.x);
	//float fy = step(0.5, a.y);

	//FragColor.rgb = mix(mix(J, L, fy), mix(K, M, fy), fx);
	
}	
}

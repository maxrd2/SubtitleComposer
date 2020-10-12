#!/usr/bin/node --trace-uncaught

const util = require('util');

Object.prototype.lp = function(name, value) {
	Object.defineProperty(this, name, { value: value });
	return value;
}

Number.prototype.toStdFloat = function(precision=7){
	return this.toPrecision(precision).replace(/([^\.])0+$/, '$1');
}

Number.prototype.toCFloat = function(precision=7){
	return this.toStdFloat(precision) + (precision <= 7 ? 'f' : '');
}

class vec {
	constructor(x, y, z, w) {
		let a = [];
		for(const i in arguments) {
			const v = arguments[i];
			if(v instanceof Array)
				a = a.concat(v);
			else if(v instanceof vec)
				a = a.concat(v.elem);
			else if(typeof(v) == 'number')
				a.push(v);
		}
		this.elem = a.slice(0, 4);
		this.ch = ['x', 'y', 'z', 'w'].slice(0, this.elem.length);
		// dynamic property array getter - inst.xyz, inst.xyzz, ... will get
		const rgb = { x:'r', y:'g', z:'b', w:'a'};
		for(let i = 0, pr = ['']; i < this.elem.length; i++) {
			for(let j = 0, n = pr.length; j < n; j++) {
				const pre = pr.shift();
				for(let k = 0; k < this.elem.length; k++) {
					const key = pre + this.ch[k];
					pr.push(key);
					if(pre)
						Object.defineProperty(this, key, { get: () => this.val(key) });
					else
						this[key] = this.elem[k];
					Object.defineProperty(this, key.replace(/./g, (m) => rgb[m]), { get: () => this.val(key) });
				}
			}
		}
	}

	val(name) { return name.length > 1 ? name.split('').map(v => this[v]) : this[name]; }

	get size() { return this.elem.length; }

	get line() { return { a:-this.y, b:this.x, c:0 }; }

	get mag() { return this.lp('mag', Math.sqrt(Math.pow(this.x, 2) + Math.pow(this.y, 2))); }

	debugDump(name, comment, precision=7) {
		if(comment)
			console.log(`// ${comment}`);
		const p = precision ? (val) => val.toFixed(precision).replace(/^-(0\.0+)$/, '$1') : (val) => val;
		console.log(`${name}\t${this.elem.map(v => p(v)).join('\t')}`);
	}

	angle(other) {
		const l1 = this.line;
		const l2 = other.line;
		const m1 = -l1.a / l1.b;
		const m2 = -l2.a / l2.b;
		return Math.atan((m1 - m2) / (1. + m1 * m2));
	}

	dot(other) {
		let res = 0;
		for(let i = 0; i < this.size; i++) {
			const ch = String.fromCharCode(120 + i);
			res += this[ch] * other[ch];
		}
		return res;
		// return this.mag * other.mag * Math.cos(this.angle(other));
		// return this.x * other.x + this.y * other.y;
	}

	mul(other) { return new vec(this.elem.map(v => v * other)); }
	div(other) { return new vec(this.elem.map(v => v / other)); }

	/**
	 * intersection point of line p1-p2 and line p3-p4
	 */
	static intersection(p1, p2, p3, p4) {
		const x12 = p1.x - p2.x;
		const x34 = p3.x - p4.x;
		const y12 = p1.y - p2.y;
		const y34 = p3.y - p4.y;
		const d = x12 * y34 - y12 * x34;
		const xy12 = p1.x * p2.y - p1.y * p2.x;
		const xy34 = p3.x * p4.y - p3.y * p4.x;
		const ix = (xy12 * x34 - x12 * xy34) / d;
		const iy = (xy12 * y34 - y12 * xy34) / d;
		return new vec(ix, iy);
	}

	/**
	 * line through two points [ax + by + c = 0]
	 */
	static line(x1, y1, x2, y2) {
		return {
			a: y1 - y2,
			b: x2 - x1,
			c: x1 * y2 - x2 * y1,
		};
	}
}

class mat {
	constructor(n_or_array) {
		if(arguments.length > 1) {
			this.elem = Array.from(arguments);
		} else if(n_or_array instanceof Array) {
			this.elem = n_or_array;
		} else if(typeof(n_or_array) == 'number'){
			this.elem = Array.from(new Array(n_or_array * n_or_array))
				.map((v, i) => i % n_or_array == parseInt(i / n_or_array) ? 1. : 0.);
		} else {
			throw `mat() should be constructed by numbers list, numbers array or mat size`;
		}
	}

	get length() { return this.elem.length; }
	get size() { return this.lp('size', Math.sqrt(this.length)); }

	array2D(precision) {
		let s = [];
		const p = precision ? (val) => parseFloat(val.toPrecision(precision)) : (val) => val;
		for(let y = 0; y < this.size; y++) {
			s[y] = [];
			for(let x = 0; x < this.size; x++)
				s[y].push(p(this.elem[y * this.size + x]));
		}
		return s;
	}
	debugDump(name, comment, precision=7) {
		let s = [];
		const p = precision ? (val) => val.toFixed(precision).replace(/^-(0\.0+)$/, '$1') : (val) => val;
		for(let y = 0; y < this.size; y++) {
			s[y] = [];
			for(let x = 0; x < this.size; x++)
				s[y].push(p(this.elem[y * this.size + x]));
			s[y] = s[y].join('\t');
		}
		if(comment)
			console.log(`// ${comment}`);
		console.log(`${name}\t${s.join('\n\t')}`);
	}
	glDump(name, comment, precision=7) {
		let s = [];
		for(let y = 0; y < this.size; y++)
			for(let x = 0; x < this.size; x++)
				s.push((!x ? '\n\t' : '') + this.elem[y * this.size + x].toStdFloat(precision));
		console.log(`mat${this.size} ${name} = mat${this.size}(${s.join(', ')});`);
	}
	cFloat(name, comment, precision=7) {
		let s = [];
		for(let y = 0; y < this.size; y++)
			for(let x = 0; x < this.size; x++)
				s.push(this.elem[y + x * this.size].toCFloat(precision));
		return { name:name, comment:comment, mat:s };
	}

	mul(other) {
		return typeof(other) == 'number' ? this.mulScalar(other) :
			  (other instanceof vec ? this.mulVec(other) : this.mulMat(other));
	}
	mulVec(other) {
		if(other.size != this.size)
			throw `mat${this.size} and vec${other.size} must have same size`;
		let res = other.elem.map(() => 0.);
		for(let r = 0; r < this.size; r++) {
			const ro = r * this.size;
			for(let c = 0; c < other.size; c++)
				res[r] += other.elem[c] * this.elem[ro + c];
		}
		return new vec(res);
	}
	mulMat(other) {
		if(other.size != this.size)
			throw `mat${this.size} and mat${other.size} must have same size`;
		let res = [];
		for(let r = 0; r < this.size; r++) {
			for(let c = 0; c < this.size; c++) {
				let v = 0.;
				for(let k = 0; k < this.size; k++)
					v += this.elem[r * this.size + k] * other.elem[k * this.size + c];
				res.push(v);
			}
		}
		return new mat(res);
	}
	mulScalar(other) {
		return new mat(this.elem.map((v, i) => v * other));
	}

	trans() {
		return new mat(this.elem.map((v, i) => this.elem[parseInt(i / this.size) + (i % this.size) * this.size]));
	}

	sum(other) {
		if(other.size != this.size)
			throw `mat${this.size} and mat${other.size} must have same size`;
		return new mat(this.elem.map((v, i) => v + other.elem[i]));
	}

	det() {
		let el = this.elem.map(v => v);
		for(let i = 0; i < this.size - 1; i++) {
			for(let j = i + 1; j < this.size; j++) {
				const mul = el[j * this.size + i] / el[i * this.size + i];
				if(!mul || isNaN(mul)) continue;
				for(let k = i; k < this.size; k++)
					el[j * this.size + k] -= mul * el[i * this.size + k];
			}
		}
		let d = 1.;
		for(let i = 0; i < this.size; i++)
			d *= el[this.size * i + i];
		return d;
	}

	inv() {
		let m1 = [];
		for(let i = 0; i < this.size; i++) {
			let s = this.elem.slice(i * this.size, (i + 1) * this.size);
			for(let k = 0; k < this.size; k++)
				s.push(i == k ? 1. : 0.);
			m1.push(s);
		}
		mat.solveLinearEquation(m1);
		for(let i = 0; i < this.size; i++)
			m1[i] = m1[i].slice(this.size);
		return new mat(m1.flat());
	}

	static solveLinearEquation(sys) {
		const len = Math.min(sys.length, sys[0].length - 1);
		for(let j = 0; j < len; j++) {
			// normalize diagonal
			for(let i = 0, n = sys[j][j]; i < sys[j].length; i++)
				sys[j][i] /= n;
			// zero above
			for(let jj = j - 1; jj >= 0; jj--) {
				for(let i = j, n = sys[jj][j]; i < sys[j].length; i++)
					sys[jj][i] -= n * sys[j][i];
			}
			// zero under
			for(let jj = j + 1; jj < sys.length; jj++) {
				for(let i = j, n = sys[jj][j]; i < sys[j].length; i++)
					sys[jj][i] -= n * sys[j][i];
			}
		}
		return sys.map(el => el[el.length - 1]);
	}
}

class ColorCoefficients {
	// NOTE: fullRange is ignored... matrix is always full range
	constructor(Kr, Kg, Kb, fullRange) {
		this.Kr = Kr;
		this.Kg = Kg;
		this.Kb = Kb;
		this.fullRange = fullRange;
	}

	mat3_RGB_RGB() {
		return new mat(
			1., 0., 0.,
			0., 1., 0.,
			0., 0., 1.
		);
	}

	// Y'CbCr (full range) and Y'PbPr
	mat3_YUV_RGB() {
		/*
			Rec. 709 calls for luma range 0...219, offset +16 at the interface. Cb and Cr are
			scaled individually to range ±112, an excursion 224/219 of luma, offset +128 at
			the interface. Codes 0 and 255 are prohibited.
		*/

		return new mat(
			1.0,	0.0,											2.0 * (1.0 - this.Kr),
			1.0,	-2.0 * (1.0 - this.Kb) * (this.Kb / this.Kg),	-2.0 * (1.0 - this.Kr) * (this.Kr / this.Kg),
			1.0,	2.0 * (1.0 - this.Kb),							0.0
		);

		/*
		const Ymax = this.fullRange ? 255.0 / 256.0 : (16.0 + 219.0) / 254.0;
		const Cmax = this.fullRange ? 255.0 / 256.0 : (16.0 + 224.0) / 254.0;
		const Umax = (Cmax - this.Kb) / 2.0; // (B'max - Y'(B'max)) / 2.0
		const Vmax = (Cmax - this.Kr) / 2.0; // (R'max - Y'(R'max)) / 2.0

//		const R = (255 / 219) * (Y - 16)																			+ (255 / 224) * 2 * (1. - this.Kr)                       * (Cr - 128);
//		const G = (255 / 219) * (Y - 16)	- (255 / 224) * 2 * (1. - this.Kb) * (this.Kb / this.Kg) * (Cb - 128)	- (255 / 224) * 2 * (1. - this.Kr) * (this.Kr / this.Kg) * (Cr - 128);
//		const B = (255 / 219) * (Y - 16)	+ (255 / 224) * 2 * (1. - this.Kb)                       * (Cb - 128);

		return new mat(
			1., 0., (1. - this.Kr) / Vmax,
			1., -this.Kb * (1. - this.Kb) / (Umax * this.Kg), -this.Kr * (1. - this.Kr) / (Vmax * this.Kg),
			1., (1. - this.Kb) / Umax, 0.
		);
		*/
	}
}

class ColorPrimaries {
	/**
	 * Params are CIE xy (not normalized) chromacity diagram coordinates - ISO 23001-8-18 (7.1) Color primaries
	 */
	constructor(Rx, Ry, Gx, Gy, Bx, By, Wx, Wy) {
		// ColourPrimaries indicates the chromaticity coordinates of the source colour primaries
		// in terms of the CIE 1931 definition of x and y as specified by ISO 11664-1.
		this.Rx = Rx;
		this.Ry = Ry;
		this.Rz = 1 - Rx - Ry;
		this.Gx = Gx;
		this.Gy = Gy;
		this.Gz = 1 - Gx - Gy;
		this.Bx = Bx;
		this.By = By;
		this.Bz = 1 - Bx - By;

		// CIE chromaticity coordinates (Wx, Wy) represent white point of standardized illuminant of a perfectly
		// reflecting (or transmitting) diffuser. The CIE chromaticity coordinates are given for the 2 degree field
		// of view (1931). The color swatches represent the hue and RGB of white point, calculated with luminance
		// Y=0.54 and the standard observer, assuming correct sRGB display calibration.
		// We can use Bradford Matrix to convert XYZ to different (Wx, Wy)
		this.Wx = Wx;
		this.Wy = Wy;
		this.Wz = 1 - Wx - Wy;
	}

	/**
	 * Calculate and return RGB Luminance coefficients
	 */
	get rgbYratios() {
		// https://stackoverflow.com/questions/53952959/why-were-the-constants-in-yuv-rgb-chosen-to-be-the-values-they-are/64392724#64392724
		// Digital Video and HDTV - Algorithms and Interfaces - C. Poynton (2003) WW - Luminance coefficients - pg.256
		return this.lp('rgbYratios', new vec(mat.solveLinearEquation([
			// Ra, Ga, Ba, W
			[ this.Rx, this.Gx, this.Bx, this.Wx / this.Wy ],
			[ this.Ry, this.Gy, this.By, this.Wy / this.Wy ],
			[ this.Rz, this.Gz, this.Bz, this.Wz / this.Wy ],
		])));
	}

	/**
	 * mix of multiple CIE (x, y) chromaticities with Y luminance
	 */
	mix(x1, y1, Y1, ...args) {
		if(arguments.length % 3)
			throw 'Wrong number of components... need multiple CIE xyY coordinates';

		/*
			https://en.wikipedia.org/wiki/CIE_1931_color_space

			The CIE XYZ color space was deliberately designed so that the Y parameter is a measure
			of the luminance of a color. The chromaticity is then specified by the two derived parameters x and y,
			two of the three normalized values being functions of all three tristimulus values X, Y, and Z:

			x = X / (X + Y + Z)
			y = Y / (X + Y + Z)
			z = Z / (X + Y + Z) = 1 - x - y

			The X and Z tristimulus values can be calculated back from the chromaticity values x and y and the Y tristimulus value:

			X = Y * x / y
			Z = Y * z / y = Y * (1 - x - y) / y

			When two or more colors are additively mixed, the x and y chromaticity coordinates of the resulting color (xMix, yMix)
			may be calculated from the chromaticities of the mixture components (x1,y1; x2,y2; ...; xn,yn) and their corresponding
			luminances (L1, L2, ..., Ln) with the following formulas:

			xMix = (x1/y1*L1 + x2/y2*L2 + x3/y3*L3 + ...) / (L1/y1 + L2/y2 + L3/y3 + ...)
			yMix = (L1 + L2 + L3 + ...) / (L1/y1 + L2/y2 + L3/y3 + ...)
			zMix = 1 - xMix - yMix; // derived from below
			... Ln == Yn

			These formulas can be derived from the previously presented definitions of x and y chromaticity coordinates by taking
			advantage of the fact that the tristimulus values X, Y, and Z of the individual mixture components are directly additive.

			In place of the luminance values (L1, L2, etc.) one can alternatively use any other photometric quantity that is directly
			proportional to the tristimulus value Y (naturally meaning that Y itself can also be used as well).

			As already mentioned, when two colors are mixed, the resulting color (xMix, yMix) will lie on the straight line segment
			that connects these colors on the CIE xy chromaticity diagram. To calculate the mixing ratio of the component colors (x1, y1)
			and (x2, y2) that results in a certain (xMix, yMix) on this line segment, one can use the formula:

			L1 / L2 = (y1 * (x2 - xMix)) / (y2 * (xMix - x1)) = (y1 * (y2 - yMix)) / (y2 * (yMix - y1))

			Note that, in accordance with the remarks concerning the formulas for xMix and yMix, the mixing ratio L1/L2 may well be
			expressed in terms of photometric quantities other than luminance.
		*/

		let div = 0, xMix = 0, yMix = 0;
		for(let i = 0; i < arguments.length; i += 3) {
			const x = arguments[i + 0];
			const y = arguments[i + 1];
			const Y = arguments[i + 2];
			div += Y / y;
			xMix += Y * x / y;
			yMix += Y;
		}
		xMix /= div;
		yMix /= div;

		return new vec(xMix, yMix, 1. - xMix - yMix);
	}

	mat3_YUV_RGB() {
		// YUV -> RGB coefficients
		const rat = this.rgbYratios;
		const Kr = rat.r * this.Ry;
		const Kg = rat.g * this.Gy;
		const Kb = rat.b * this.By;

		const sum = parseFloat((Kr + Kg + Kb).toFixed(6));
		if(sum !== 1.0)
			console.warn(`${Kr} + ${Kg} + ${Kb} != 1.0 (${sum})`);

		return new ColorCoefficients(Kr, Kg, Kb, false).mat3_YUV_RGB();
	}

	/**
	 * Get luminance Y at CIE (x, y) chromaticity
	 */
	get_xy_Y(pt) {
		const rat = this.rgbYratios;
		const Rv = new vec(this.Rx, this.Ry, rat.r * this.Ry);
		const Gv = new vec(this.Gx, this.Gy, rat.g * this.Gy);
		const Bv = new vec(this.Bx, this.By, rat.b * this.By);

		// luminance between p1 and p2 at position x - x must be at line p1-p2
		const lumaAtCIExy = (p1, p2, xMix) => {
			const a = p1.y * (p2.x - xMix);
			const b = p2.y * (xMix - p1.x);

			return p1.z * a / (a + b) + p2.z * b / (a + b);

			if(Math.abs(a) <= 1e-5) // xMix is at p2
				return p2.z;
			if(Math.abs(b) <= 1e-5) // xMix is at p1
				return p1.z;
			return p2.z * a / b + p1.z * b / a;
		};

		// p - intersection point between R-G and B-(x, y)
		let ip = vec.intersection(Rv, Gv, Bv, pt);
		if(Number.isNaN(ip.x)) // can't make line between Bv and (x, y) cause they're same point
			return Bv.z;
		// luminance at R-G intersection point
		ip = new vec(ip.xy, lumaAtCIExy(Rv, Gv, ip.x));
		// luminance at (x, y)
		return lumaAtCIExy(ip, Bv, pt.x);
	}

	// approximation at best - they are not linear transforms
	mat4_xyY_RGB() {
		return this.mat4_RGB_xyY().inv();
	}
	mat4_RGB_xyY() {
		const rat = this.rgbYratios;
		const cx = new vec(mat.solveLinearEquation([
			[ 1, 1, 1, 1, this.Wx ],
			[ 1, 0, 0, 1, this.Rx ],
			[ 0, 1, 0, 1, this.Gx ],
			[ 0, 0, 1, 1, this.Bx ],
		]));
		const cy = new vec(mat.solveLinearEquation([
			[ 1, 1, 1, 1, this.Wy ],
			[ 1, 0, 0, 1, this.Ry ],
			[ 0, 1, 0, 1, this.Gy ],
			[ 0, 0, 1, 1, this.By ],
		]));
		return new mat(
			cx.r, cx.g, cx.b, cx.a,
			cy.r, cy.g, cy.b, cy.a,
			rat.r * this.Ry, rat.g * this.Gy, rat.b * this.By, 0.,
			0., 0., 0., 1.
		);
	}

	mat3_XYZ_RGB() { return this.mat3_RGB_XYZ().inv(); }
	mat3_RGB_XYZ() {
		const rat = this.rgbYratios;
		return new mat(
			rat.r * this.Rx, rat.g * this.Gx, rat.b * this.Bx,
			rat.r * this.Ry, rat.g * this.Gy, rat.b * this.By,
			rat.r * this.Rz, rat.g * this.Gz, rat.b * this.Bz
		);
	}
}

class ColorTransfer {
	constructor(funcs, values) {
		/*
			ISO 23001-8-18 (7.2) Transfer characteristics
			TransferCharacteristics indicates the opto-electronic transfer characteristic of the source picture,
			as specified in Table 3, as a function of a linear optical intensity input Lc with a nominal real-valued
			range, of 0 to 1. For interpretation of entries in Table 3 that are expressed in terms of multiple curve
			segments parameterized by the variable α over a region bounded by the variable β or by the variables
			β and γ, the values of α and β are defined to be the positive constants necessary for the curve segments
			that meet at the value β to have continuity of both value and slope at the value β. The value of γ, when
			applicable, is defined to be the positive constant necessary for the associated curve segments to meet at
			the value γ.
			For example, for TransferCharacteristics equal to 1, 6, 14, or 15, β has the value 0.018053968510807, and
			α has the value 1 + 5.5 * β = 1.099296826809442.
		*/

		this.funcs = funcs.map((v, i) => {
			for(const val in values)
				v = String(v).replace(new RegExp(`(?<=\\W|^)${val}(?=\\W|$)`, 'g'), `(${values[val]})`);
			return this.funcToTree(v);
		});
	}

	funcString(tree, fMap, pre, post, first) {
		if(typeof(tree) === 'number')
			return fMap.number(tree);
		if(typeof(tree) === 'string')
			return this.isOperator(tree) && !first ? ` ${tree} ` : tree;
		if(tree.func)
			return fMap[tree.func].apply(this, tree.args.map(v => this.funcString(v, fMap)));

		let str = '';
		for(let i = 0; i < tree.length; i++)
			str += this.funcString(tree[i], fMap, tree[i - 1], tree[i + 1], i < 1);
		return tree.length > 1 && ((pre && pre != '+') || post == '*' || post == '/') ? `(${str})` : str;
	}

	funcGLSL(tree) {
		return 'return ' + this.funcString(tree, {
			number: v => v.toStdFloat(7),
			pow: (b, e) => `pow(${b}, ${e})`,
			log: v => `(log(${v}) / log(10.0))`
		}) + ';\n';
	}

	treeReplaceVar(tree, name, val) {
		if(Array.isArray(tree)) {
			for(let i = 0; i < tree.length; i++)
				tree[i] = this.treeReplaceVar(tree[i], name, val);
			return tree;
		}
		if(tree.func) {
			tree.args = this.treeReplaceVar(tree.args, name, val);
			return tree;
		}
		return tree === name ? val : tree;
	}

	treeContains(tree, name) {
		if(Array.isArray(tree)) {
			for(let i = 0; i < tree.length; i++) {
				if(this.treeContains(tree[i], name))
					return true;
			}
			return false;
		} else if(tree.func) {
			return this.treeContains(tree.args, name);
		}

		return tree === name;
	}

	funcExtract(tree, name) {
//		const ui = v => util.inspect(v, false, null, false);
//		console.log(`***** funcExtract(${name})`, ui(tree));
		tree = JSON.parse(JSON.stringify(tree)); // deep clone
		let right = [];
		const funcInv = {
			pow: (l, r) => [ l[0], { func:'pow', args:[r, this.treeSimplify([1.0, '/', l[1]])] } ],
			log: (l, r) => [ l[0], { func:'pow', args:[10, this.treeSimplify(r)] } ],
		};

		for(;;) {
			if(tree.length == 1 && tree[0] === name)
				break;

			let namePos = -1;
			for(let i = 0; i < tree.length; i++) {
				if(this.treeContains(tree[i], name)) {
					namePos = i;
					break;
				}
			}

			if(namePos < 0) {
				console.warn(`WARNING: variable '${name}' is not in equation.`);
				return [0.0];
			}

			if(namePos == 0) {
				if(tree.length > 1) {
					let oper = tree[1];
					switch(oper) {
					case '-':
						right = this.treeSimplify(right, true);
						right = this.treeSimplify([right, '+', this.treeSimplify(tree.splice(2, tree.length - 2))], true);
						break;
					case '+':
						right = this.treeSimplify(right, true);
						right = this.treeSimplify([right, '+', this.treeNegate(tree.splice(2, tree.length - 2))], true);
						break;
					case '*':
						right = this.treeSimplify(right, true);
						right = this.treeSimplify([right, '/', this.treeSimplify(tree.splice(2, tree.length - 2))], true);
						break;
					case '/':
						right = this.treeSimplify(right, true);
						right = this.treeSimplify([right, '*', this.treeSimplify(tree.splice(2, tree.length - 2))], true);
						break;
					default:
						oper = null;
						break;
					}
					if(oper) {
						tree.splice(1, 1);
						tree = this.treeSimplify(tree, true);
//						console.log(`post${oper}`, ui(tree), '==', ui(right));
					} else {
						throw `Unexpected operator ${tree[1]}`;
					}
				}
				if(tree[0] === name)
					break;
				if(tree[0].func) {
					const f = tree[0].func;
//					console.log('INV', tree[0], ' == ', ui(right));
					const r = funcInv[f](tree[0].args, right);
					tree[0] = r[0];
					tree = this.treeSimplify(tree, true);
					right = [r[1]];
//					console.log(`inv-${f}`, ui(tree), '==', ui(right));
				}
			} else {
				let oper = tree[namePos - 1];
				switch(oper) {
				case '-':
					if(namePos == 1) {
						right = this.treeNegate(right, true);
					} else {
						right = this.treeSimplify(right, true);
						right = this.treeSimplify([right, '+', this.treeSimplify(tree.splice(0, namePos - 1))], true);
					}
					break;
				case '+':
					right = this.treeSimplify(right, true);
					right = this.treeSimplify([right, '+', this.treeNegate(tree.splice(0, namePos - 1))], true);
					break;
				case '*':
					right = this.treeSimplify(right, true);
					right = this.treeSimplify([right, '/', this.treeSimplify(tree.splice(0, namePos - 1))], true);
					break;
				case '/':
					right = this.treeSimplify(right, true);
					right = this.treeSimplify([this.treeSimplify(tree.splice(0, namePos - 1)), '/', right], true);
					break;
				default:
					oper = null;
					break;
				}
				if(oper) {
					tree.splice(0, 1);
					tree = this.treeSimplify(tree, true);
//					console.log(`pre${oper}`, ui(tree), '==', ui(right));
				}
			}
		}

		right = this.treeSimplify(right);
//		console.log('done:', ui(tree), '==', ui(right));

		return right;
	}

	isOperator(val) { return ['*', '/', '+', '-'].includes(val); }

	funcParams(str) {
		let params = [];

		while(str && str[0] != ')') {
			let r = this.funcProc(str);
			str = r[1];
			if(r[0])
				params.push(r[0]);
			if(r[2] == ')')
				break;
		}

		return [params, str];
	}

	treeNegate(tree, toplevel) {
		if(typeof(tree) === 'number')
			return -tree;
		if(tree[0] == '-') {
			tree.splice(0, 1);
			return this.treeSimplify(tree, toplevel);
		}
		return ['-', tree];
	}

	treeSimplify(tree, toplevel) {
		if(!Array.isArray(tree))
			return tree;
		tree = tree.map(v => typeof(v) == 'string' && v.match(/^[0-9\.]+$/) ? parseFloat(v) : v);
//		tree = tree.map(v => Array.isArray(v) ? this.treeSimplify(v) : v);

		while((Array.isArray(tree[0]) && !tree[0].length) || tree[0] === '+')
			tree.shift();

		for(let i = 0; i < tree.length; i++) {
			if((i == 0 || this.isOperator(tree[i - 1])) && tree[i] === '-') {
				tree.splice(i, 1);
				tree[i] = this.treeNegate(tree[i]);
			} else if(i && Array.isArray(tree[i]) && tree[i][0] === '-') {
				if(tree[i - 1] === '-') {
					tree[i - 1] = '+';
					tree[i].splice(0, 1);
					tree[i] = this.treeSimplify(tree[i]);
				} else if(tree[i - 1] === '+') {
					tree[i - 1] = '-';
					tree[i].splice(0, 1);
					tree[i] = this.treeSimplify(tree[i]);
				}
			}
		}

		const replaceConst = (tree, toplevel) => {
			['*', '/', '+', '-'].forEach(oper => {
				for(let i = 1; i < tree.length; i++) {
					if(tree[i] !== oper || typeof(tree[i - 1]) !== 'number' || typeof(tree[i + 1]) !== 'number')
						continue;
					switch(oper) {
					case '*': tree[i - 1] *= tree[i + 1]; break;
					case '/': tree[i - 1] /= tree[i + 1]; break;
					case '+': tree[i - 1] += tree[i + 1]; break;
					case '-': tree[i - 1] -= tree[i + 1]; break;
					}
					tree.splice(i, 2);
					i = 1;
				}
			});
			return toplevel || tree.length != 1 ? tree : tree[0];
		};

		['*', '/', '+', '-'].forEach(oper => {
			for(;;) {
				let found = false;
				for(let i = 1; i < tree.length - 1;) {
					if(tree[i] !== oper) {
						i++;
						continue;
					}
					found = true;
					tree.splice(i - 1, 0, replaceConst(tree.splice(i - 1, 3)));
				}
				if(!found)
					break;
			}
		});

		tree = replaceConst(tree, toplevel);

		while(tree && (!toplevel || Array.isArray(tree[0])) && tree.length == 1)
			tree = tree[0];

		return tree;
	}

	funcProc(str) {
		let tree = [];
		let part = '';
		let ch = null;
		while(str) {
			ch = str[0]; str = str.substr(1);
			if(ch == '(') {
				if(part) {
					let r = this.funcParams(str);
					str = r[1];
					tree.push({func:part, args:r[0]});
					part = '';
				} else {
					let r = this.funcProc(str);
					str = r[1];
					tree.push(r[0]);
				}
			} else if(ch == ')' || ch == ',') {
				break;
			} else if(this.isOperator(ch)) {
				if(part)
					tree.push(part);
				part = '';
				tree.push(ch);
			} else {
				part += ch;
			}
		}
		if(part)
			tree.push(part);
		return [this.treeSimplify(tree), str, ch];
	}

	funcToTree(str) {
		return this.funcProc(str.replace(/=(.+)$/, '-($1)').replace(/\s+/g, ''))[0];
	}

	treeCalc(tree) {
		if(Array.isArray(tree)) {
			tree.forEach((v, i) => tree[i] = this.treeCalc(v));
			if(tree[0] == '-') {
				tree.splice(0, 1);
				tree[0] = -tree[0];
			}

			for(let i = 1; i < tree.length; i++) {
				if(!this.isOperator(tree[i]) || typeof(tree[i - 1]) !== 'number' || typeof(tree[i + 1]) !== 'number')
					throw 'houston we have a problem';
				switch(tree[i]) {
				case '*': tree[i - 1] *= tree[i + 1]; break;
				case '/': tree[i - 1] /= tree[i + 1]; break;
				case '+': tree[i - 1] += tree[i + 1]; break;
				case '-': tree[i - 1] -= tree[i + 1]; break;
				}
				tree.splice(i, 2);
				i = 1;
			}

			return tree.length == 1 ? tree[0] : tree;
		}
		if(tree.func) {
			const func = {
				pow: (b, e) => Math.pow(b, e),
				log: v => Math.log(v)
			};
			return func[tree.func].apply(null, tree.args.map(v => this.treeCalc(v)));
		}
		return tree;
	}

	glslShader(comment, precision=7) {
		let res = { func:'', funcInv:'', comment:comment };

		for(let i = this.funcs.length - 1; i >= 0; i -= 2) {
			if(i) {
				const l1 = this.treeCalc(this.funcs[i - 1]).toStdFloat(7);
				res.func += `if(vLin < ${l1}) `;
				const l2 = this.treeCalc(this.treeReplaceVar(this.funcExtract(this.funcs[i], 'vExp'), 'vLin', this.funcs[i - 1])).toStdFloat(7);
				res.funcInv += `if(vExp < ${l2}) `;
			}
			res.func += this.funcGLSL(this.funcExtract(this.funcs[i], 'vExp'));
			res.funcInv += this.funcGLSL(this.funcExtract(this.funcs[i], 'vLin'));
		}
		res.func = res.func.trim();
		res.funcInv = res.funcInv.trim();

//		res.debug = this.funcs;
//		const ui = v => util.inspect(v, false, null, false);
//		console.log(ui(res));

		return res;
	}
}

const sRGB = new ColorPrimaries(0.640, 0.330, 0.300, 0.600, 0.150, 0.060, 0.31271, 0.32902);

if(0){
	const n = -1/3;
	let eq = [
		[ Math.pow(sRGB.Wx / sRGB.Wy, n), sRGB.Wx, sRGB.Wy, 1.0 ],
		[ Math.pow(sRGB.Rx / sRGB.Ry, n), sRGB.Rx, sRGB.Ry, 0.21263682167732387 ],
		[ Math.pow(sRGB.Gx / sRGB.Gy, n), sRGB.Gx, sRGB.Gy, 0.7151829818412506 ],
		[ Math.pow(sRGB.Bx / sRGB.By, n), sRGB.Bx, sRGB.By, 0.07218019648142547 ], // test value
	];
	console.log('xy max Luma', eq);
	mat.solveLinearEquation(eq);
	console.log('xy max Luma', eq); // bad - not linear
	process.exit();

}

if(0){
	const dump = (name, arr) => {
		let s = [];
		const p = (val) => parseFloat(val.toFixed(3).replace(/^-(0\.0+)$/, '$1')) || '.';
		for(let y = 0; y < arr.length; y++) {
			s[y] = [];
			for(let x = 0; x < arr[y].length; x++)
				s[y].push(p(arr[y][x]));
			s[y] = s[y].join('\t');
		}
		console.log(`${name}\t${s.join('\n\t')}`);
	};

	const dist = (v1, v2) => Math.sqrt(Math.pow(v1.x - v2.x, 2) + Math.pow(v1.y - v2.y, 2));
	const RGB_XYZ = sRGB.mat3_RGB_XYZ();
	const Rv = new vec(sRGB.Rx, sRGB.Ry, RGB_XYZ.mul(new vec(1, 0, 0)).y);
	const Gv = new vec(sRGB.Gx, sRGB.Gy, RGB_XYZ.mul(new vec(0, 1, 0)).y);
	const Bv = new vec(sRGB.Bx, sRGB.By, RGB_XYZ.mul(new vec(0, 0, 1)).y);
	const Wv = new vec(sRGB.Wx, sRGB.Wy, RGB_XYZ.mul(new vec(1, 1, 1)).y);

	const xyY = (r,g,b) => {
		const v = RGB_XYZ.mul(new vec(1, 1, 0));
		const sum = v.elem.reduce((a, v) => a + v, 0);
		return new vec(v.x/sum, v.y/sum, v.y);
	};

	const RGv = xyY(1, 1, 0);
	const RBv = xyY(1, 0, 1);
	const GBv = xyY(0, 1, 1);

	const fromRGB = (r, g, b) => {
		const rat = sRGB.rgbYratios;
		return r * rat.r * sRGB.Ry
			+ g * rat.g * sRGB.Gy
			+ b * rat.b * sRGB.By;
	};

	console.log();
	console.log('red Y', fromRGB(1, 0, 0), Rv.z,  sRGB.get_xy_Y(new vec(Rv.x, Rv.y)));
	console.log('rg  Y', fromRGB(1, 1, 0), RGv.z, sRGB.get_xy_Y(new vec(RGv.x, RGv.y)));
	console.log('grn Y', fromRGB(0, 1, 0), Gv.z,  sRGB.get_xy_Y(new vec(Gv.x, Gv.y)));
	console.log('gb  Y', fromRGB(1, 1, 0), GBv.z, sRGB.get_xy_Y(new vec(GBv.x, GBv.y)));
	console.log('blu Y', fromRGB(0, 0, 1), Bv.z,  sRGB.get_xy_Y(new vec(Bv.x, Bv.y)));
	console.log('rb  Y', fromRGB(1, 1, 0), RBv.z, sRGB.get_xy_Y(new vec(RBv.x, RBv.y)));
	console.log('wht Y', fromRGB(1, 1, 1), Wv.z,  sRGB.get_xy_Y(new vec(Wv.x, Wv.y)));
	process.exit();
}

if(0) { // test CIE diagram GLSL
	console.log(`GLfloat pR[]{ ${sRGB.Rx.toCFloat()}, ${sRGB.Ry.toCFloat()}, ${(sRGB.Ry * sRGB.rgbYratios.r).toCFloat()} };`);
	console.log(`GLfloat pG[]{ ${sRGB.Gx.toCFloat()}, ${sRGB.Gy.toCFloat()}, ${(sRGB.Gy * sRGB.rgbYratios.g).toCFloat()} };`);
	console.log(`GLfloat pB[]{ ${sRGB.Bx.toCFloat()}, ${sRGB.By.toCFloat()}, ${(sRGB.By * sRGB.rgbYratios.b).toCFloat()} };`);
	console.log(`GLfloat pW[]{ ${sRGB.Wx.toCFloat()}, ${sRGB.Wy.toCFloat()}, 1.0f };`);
	console.log(`GLfloat csXYZ_RGB[]{ ${sRGB.mat3_XYZ_RGB().cFloat('XYZ_sRGB', 'XYZ -> sRGB colorspace').mat.join(', ')} };`);
	console.log(`GLfloat csxyY_RGB[]{ ${sRGB.mat4_xyY_RGB().cFloat('xyY_sRGB', 'xyY -> sRGB colorspace').mat.join(', ')} };`);
	console.log(`GLfloat triangle[]{ ${sRGB.Rx}f, ${sRGB.Ry}f, ${sRGB.Gx}f, ${sRGB.Gy}f, ${sRGB.Bx}f, ${sRGB.By}f, ${sRGB.Wx}f, ${sRGB.Wy}f };`)
}

if(0) {
	const RGB_XYZ = sRGB.mat3_RGB_XYZ();
	const XYZ_RGB = sRGB.mat3_XYZ_RGB();
	const RGB_xyY = sRGB.mat4_RGB_xyY();
	const xyY_RGB = sRGB.mat4_xyY_RGB();

	if(0) {
		RGB_XYZ.mul(new vec(1, 0, 0)).debugDump(`red   XYZ`);
		RGB_XYZ.mul(new vec(0, 1, 0)).debugDump(`green XYZ`);
		RGB_XYZ.mul(new vec(0, 0, 1)).debugDump(`blue  XYZ`);
		RGB_XYZ.mul(new vec(1, 1, 1)).debugDump(`white XYZ`);
		RGB_XYZ.mul(new vec(0, 0, 0)).debugDump(`black XYZ`);

		XYZ_RGB.mul(RGB_XYZ.mul(new vec(1, 0, 0))).debugDump(`\nred   RGB`);
		XYZ_RGB.mul(RGB_XYZ.mul(new vec(0, 1, 0))).debugDump(`green RGB`);
		XYZ_RGB.mul(RGB_XYZ.mul(new vec(0, 0, 1))).debugDump(`blue  RGB`);
		XYZ_RGB.mul(RGB_XYZ.mul(new vec(1, 1, 1))).debugDump(`white RGB`);
		XYZ_RGB.mul(RGB_XYZ.mul(new vec(0, 0, 0))).debugDump(`black RGB`);

		RGB_xyY.mul(new vec(1, 0, 0, 1)).debugDump(`\nred   xyY`);
		RGB_xyY.mul(new vec(0, 1, 0, 1)).debugDump(`green xyY`);
		RGB_xyY.mul(new vec(0, 0, 1, 1)).debugDump(`blue  xyY`);
		RGB_xyY.mul(new vec(1, 1, 1, 1)).debugDump(`white xyY`);
		RGB_xyY.mul(new vec(0, 0, 0, 1)).debugDump(`black xyY`);

		xyY_RGB.mul(RGB_xyY.mul(new vec(1, 0, 0, 1))).debugDump(`\nred   RGB`);
		xyY_RGB.mul(RGB_xyY.mul(new vec(0, 1, 0, 1))).debugDump(`green RGB`);
		xyY_RGB.mul(RGB_xyY.mul(new vec(0, 0, 1, 1))).debugDump(`blue  RGB`);
		xyY_RGB.mul(RGB_xyY.mul(new vec(1, 1, 1, 1))).debugDump(`white RGB`);
		xyY_RGB.mul(RGB_xyY.mul(new vec(0, 0, 0, 1))).debugDump(`black RGB`);
	}

	if(1) {
		xyY_RGB.mul(new vec(sRGB.Rx, sRGB.Ry, sRGB.get_xy_Y(new vec(sRGB.Rx, sRGB.Ry)), 1)).debugDump(`\nred   RGB`);
		xyY_RGB.mul(new vec(sRGB.Gx, sRGB.Gy, sRGB.get_xy_Y(new vec(sRGB.Gx, sRGB.Gy)), 1)).debugDump(`green RGB`);
		xyY_RGB.mul(new vec(sRGB.Bx, sRGB.By, sRGB.get_xy_Y(new vec(sRGB.Bx, sRGB.By)), 1)).debugDump(`blue  RGB`);
		xyY_RGB.mul(new vec(sRGB.Wx, sRGB.Wy, sRGB.get_xy_Y(new vec(sRGB.Wx, sRGB.Wy)), 1)).debugDump(`white RGB`);
	}

	if(0) {
		const rXYZ = RGB_XYZ.mul(new vec(1, 0, 0)); //.debugDump(`red   XYZ`);
		const gXYZ = RGB_XYZ.mul(new vec(0, 1, 0)); //.debugDump(`green XYZ`);
		const bXYZ = RGB_XYZ.mul(new vec(0, 0, 1)); //.debugDump(`blue  XYZ`);
		const wXYZ = RGB_XYZ.mul(new vec(1, 1, 1)); //.debugDump(`white XYZ`);

		let eq = [
			[ sRGB.Wx, sRGB.Wy, sRGB.Wz, wXYZ.y ],
			[ sRGB.Rx, sRGB.Ry, sRGB.Rz, rXYZ.y ],
			[ sRGB.Gx, sRGB.Gy, sRGB.Gz, gXYZ.y ],
			[ sRGB.Bx, sRGB.By, sRGB.Bz, bXYZ.y ], // test value
		];
		console.log('xy max Luma', eq);
		mat.solveLinearEquation(eq);
		console.log('xy max Luma', eq); // bad - not linear
		process.exit();
	}

	if(0) {
		const x1 = sRGB.Bx;
		const y1 = sRGB.By;
		const x2 = sRGB.Wx;
		const y2 = sRGB.Wy;
		const Y1 = sRGB.get_xy_Y(new vec(x1, y1));
		const Y2 = sRGB.get_xy_Y(new vec(x2, y2));
		for(let r = 0.0; r <= 1.0001; r += .1) {
			const L1 = r * Y1;
			const L2 = (1. - r) * Y2;
			const xMix = (x1/y1*L1 + x2/y2*L2) / (L1/y1 + L2/y2);
			const yMix = (L1 + L2) / (L1/y1 + L2/y2);
			const Ymix = sRGB.get_xy_Y(new vec(xMix, yMix));

			xyY_RGB.mul(new vec(xMix, yMix, Ymix, 1)).debugDump(`x:${xMix.toFixed(2)} y:${yMix.toFixed(2)} Y:${Ymix.toFixed(2)} ${r.toFixed(2)} RGB`);

			const XYZ = new vec(Ymix * xMix / yMix, Ymix, Ymix * (1 - xMix - yMix) / yMix);
			XYZ_RGB.mul(XYZ).debugDump(`\t\t     TEST RGB`);
			XYZ.debugDump(`\t\t     TEST XYZ`);
			console.log(`\t\t     TEST   L1: ${L1.toFixed(4)}\t    L2: ${L2.toFixed(4)}\t     x: ${xMix.toFixed(4)}\t     y: ${yMix.toFixed(4)}`);
		}
		console.log('whole range:');
		for(let x = 0.; x <= 1.0; x += .05)
			console.log(x, xyY_RGB.mul(new vec(x, x, 1, 1)).xyz);
	}
	process.exit();
}



// Header
console.log('#include <QMap>\n#include <QVector>\n#include <QOpenGLFunctions>\n\nnamespace SubtitleComposer {\n')

// Colorspace conversion from ISO IEC 23001-8:2018 (7.1) data
{
	const csm = {
		1: new ColorPrimaries(0.640, 0.330, 0.300, 0.600, 0.150, 0.060, 0.31271, 0.32902).mat3_YUV_RGB().cFloat('AVCOL_PRI_BT709',
			'ITU-R BT.709-5; ITU-R BT.1361; IEC 61966-2-1 sRGB or sYCC; IEC 61966-2-4; SMPTE-RP-177:1993b'),
		2: { comment: 'UNSPECIFIED - Image characteristics are unknown or are determined by the application' },
		4: new ColorPrimaries(0.67, 0.33, 0.21, 0.71, 0.14, 0.08, 0.31006, 0.31616).mat3_YUV_RGB().cFloat('AVCOL_PRI_BT470M',
			'ITU-R BT.470-6m; US-NTSC-1953; USFCCT-47:2003-73.682a'),
		5: new ColorPrimaries(0.64, 0.33, 0.29, 0.60, 0.15, 0.06, 0.31271, 0.32902).mat3_YUV_RGB().cFloat('AVCOL_PRI_BT470BG',
			'ITU-R BT.470-6bg; ITU-R BT.601-6 625; ITU-R BT.1358 625; ITU-R BT.1700 625 PAL/SECAM'),
		6: new ColorPrimaries(0.630, 0.340, 0.310, 0.595, 0.155, 0.070, 0.31271, 0.32902).mat3_YUV_RGB().cFloat('AVCOL_PRI_SMPTE170M',
			'ITU-R BT.601-6 525; ITU-R BT.1358 525; ITU-R BT.1700 NTSC; SMPTE-170M:2004'),
		7: new ColorPrimaries(0.630, 0.340, 0.310, 0.595, 0.155, 0.070, 0.31271, 0.32902).mat3_YUV_RGB().cFloat('AVCOL_PRI_SMPTE240M',
			'SMPTE-240M:1999'),
		8: new ColorPrimaries(0.681, 0.319, 0.243, 0.692, 0.145, 0.049, 0.31006, 0.31616).mat3_YUV_RGB().cFloat('AVCOL_PRI_FILM',
			'Generic film (color filters using CIE SI C)'),
		9: new ColorPrimaries(0.708, 0.292, 0.170, 0.797, 0.131, 0.046, 0.31271, 0.32902).mat3_YUV_RGB().cFloat('AVCOL_PRI_BT2020',
			'Rec. ITU-R BT.2020'),
		10: new ColorPrimaries(1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0/3.0, 1.0/3.0).mat3_YUV_RGB().cFloat('AVCOL_PRI_SMPTE428',
			'SMPTE-ST-428-1 (CIE 1931 XYZ as in ISO 11664-1)'),
		11: new ColorPrimaries(0.680, 0.320, 0.265, 0.690, 0.150, 0.060, 0.314, 0.351).mat3_YUV_RGB().cFloat('AVCOL_PRI_SMPTE431',
			'SMPTE-RP-431-2:2011'),
		12: new ColorPrimaries(0.680, 0.320, 0.265, 0.690, 0.150, 0.060, 0.31271, 0.32902).mat3_YUV_RGB().cFloat('AVCOL_PRI_SMPTE432',
			'SMPTE-EG-432-1:2010'),
		22: new ColorPrimaries(0.630, 0.340, 0.295, 0.605, 0.155, 0.077, 0.31271, 0.32902).mat3_YUV_RGB().cFloat('AVCOL_PRI_EBU3213',
			'EBU-3213-E:1975'),
	};
	console.log('// from ISO IEC 23001-8:2018 (7.1) data')
	console.log('const static QMap<int, QVector<GLfloat>> _csm{');
	const n = Object.keys(csm).reduce((ac, v) => Math.max(ac, v), 0);
	for(let i = 0; i <= n; i++) {
		const e = csm[i];
		if(e && e.mat)
			console.log(`\t// ${i} - ${e.comment}\n\t{ ${i}, QVector<GLfloat>{ ${e.mat.join(', ')} }},`);
		else
			console.log(`\t// ${i} - ${e ? e.comment : 'RESERVED - For future use by ISO/IEC'}`);
	}
	console.log('};\n');
}

// Transfer characteristics inverse from ISO IEC 23001-8:2018 (7.2) data
{
	const ctf = {
		1: new ColorTransfer(['vExp = α * pow(vLin, γ) - (α - 1)', 'β', 'vExp = lm * vLin'],
							 {'α': '1 + 5.5 * β', 'β': 0.018053968510807, 'γ': 0.45, 'lm': 4.5}).glslShader(
			   'ITU-R BT.709-5; ITU-R BT.1361'),
		2: { comment: 'Unspecified - Image characteristics are unknown or are determined by the application' },
		// Assumed display gamma 2.2 - very similar to BT.709
		4: new ColorTransfer(['vExp = α * pow(vLin, γ) - (α - 1)', 'β', 'vExp = lm * vLin'],
							 {'α': '1 + 5.5 * β', 'β': 0.018053968510807, 'γ': 1 / 2.2, 'lm': 4.5}).glslShader(
			   'ITU-R BT.470-6m; US-NTSC-1953; USFCCT-47:2003-73.682a; ITU-R BT.1700:2007 625 PAL/SECAM'),
		// Assumed display gamma 2.8 - very similar to sRGB
		5: new ColorTransfer(['vExp = α * pow(vLin, γ) - (α - 1)', 'β', 'vExp = lm * vLin'],
							 {'α': 1.055, 'β': 0.0031308, 'γ': 1 / 2.8, 'lm': 12.92}).glslShader(
			   'ITU-R BT.1700:2007 625 PAL/SECAM; ITU-R BT.470-6bg'),
		6: new ColorTransfer(['vExp = α * pow(vLin, γ) - (α - 1)', 'β', 'vExp = lm * vLin'],
							 {'α': '1 + 5.5 * β', 'β': 0.018053968510807, 'γ': 0.45, 'lm': 4.5}).glslShader(
			   'ITU-R BT.601-6 525/625; ITU-R BT.1358 525/625; ITU-R BT.1700 NTSC; SMPTE-170M:2004'),
		7: new ColorTransfer(['vExp = α * pow(vLin, γ) - (α - 1)', 'β', 'vExp = lm * vLin'],
							 {'α': 1.1115, 'β': 0.0228, 'γ': 0.45, 'lm': 4.0}).glslShader(
			   'SMPTE-240M:1999'),
		8: new ColorTransfer(['vExp = vLin'], {}).glslShader(
			   'Linear transfer characteristics'),
		9: new ColorTransfer(['vExp = 1.0 + log(vLin) / 2', 0.01, 'vExp = 0.0'], {}).glslShader(
			   'Logarithmic transfer characteristic (100:1 range)'),
		10: new ColorTransfer(['vExp = 1.0 + log(vLin) / 2.5', Math.sqrt(10) / 1000, 'vExp = 0.0'], {}).glslShader(
			   'Logarithmic transfer characteristic (100 * Sqrt(10) : 1 range)'),
		11: new ColorTransfer(['vExp = α * pow(vLin, γ) - (α - 1)', 'β', 'vExp = lm * vLin', '-β', 'vExp = -α * pow(-vLin, γ) + (α - 1)'],
							 {'α': '1 + 5.5 * β', 'β': 0.018053968510807, 'γ': 0.45, 'lm': 4.5}).glslShader(
			   'IEC 61966-2-4'),
		12: new ColorTransfer(['vExp = α * pow(vLin, ex) - (α - 1)', 'β', 'vExp = lm * vLin', '-γ', 'vExp = -(α * pow(-4 * vLin, ex) - (α - 1)) / 4'],
							 {'α': '1 + 5.5 * β', 'β': 0.018053968510807, 'γ': 0.004, 'ex': 0.45, 'lm': 4.5}).glslShader(
			   'ITU-R BT.1361'),
		13: new ColorTransfer(['vExp = α * pow(vLin, 1.0 / γ) - (α - 1)', 'β', 'vExp = lm * vLin'],
							 {'α': 1.055, 'β': 0.0031308, 'γ': 2.4, 'lm': 12.92}).glslShader(
			   'IEC 61966-2-1 sRGB/sYCC'),
		14: new ColorTransfer(['vExp = α * pow(vLin, γ) - (α - 1)', 'β', 'vExp = lm * vLin'],
							 {'α': '1 + 5.5 * β', 'β': 0.018053968510807, 'γ': 0.45, 'lm': 4.5}).glslShader(
			   'ITU-R BT.2020 (10-bit system)'),
		15: new ColorTransfer(['vExp = α * pow(vLin, γ) - (α - 1)', 'β', 'vExp = lm * vLin'],
							 {'α': '1 + 5.5 * β', 'β': 0.018053968510807, 'γ': 0.45, 'lm': 4.5}).glslShader(
			   'ITU-R BT.2020 (12-bit system)'),
		16: new ColorTransfer(['vExp = pow((c1 + c2 * pow(vLin, n)) / (1 + c3 * pow(vLin, n)), m)'],
							 {'c1': 'c3 - c2 + 1', 'c2': '2413 / 128', 'c3': '2392 / 128', 'm': '2523 / 32', 'n': '653 / 4096'}).glslShader(
			   'SMPTE-ST-2084 (for TV 10, 12, 14, and 16-bit systems)'),
		17: new ColorTransfer(['vExp = pow(48 * vLin / 52.37, 1 / 2.6)'],
							 {}).glslShader(
				'SMPTE-ST-428-1'),
	};
	console.log('// from ISO IEC 23001-8:2018 (7.2) data')
	console.log('const static QMap<int, QString> _ctf{');
	const n = Object.keys(ctf).reduce((ac, v) => Math.max(ac, v), 0);
	for(let i = 0; i <= n; i++) {
		const e = ctf[i];
		if(e && e.func)
			console.log(`\t// ${i} - ${e.comment}\n\t{ ${i}, QStringLiteral("${e.func.replaceAll('\n', '"\n\t\t\t"')}") },`);
		else
			console.log(`\t// ${i} - ${e ? e.comment : 'RESERVED - For future use by ISO/IEC'}`);
	}
	console.log('};');
	console.log('const static QMap<int, QString> _ctfi{');
	for(let i = 0; i <= n; i++) {
		const e = ctf[i];
		if(e && e.funcInv)
			console.log(`\t// ${i} - ${e.comment}\n\t{ ${i}, QStringLiteral("${e.funcInv.replaceAll('\n', '"\n\t\t\t"')}") },`);
		else
			console.log(`\t// ${i} - ${e ? e.comment : 'RESERVED - For future use by ISO/IEC'}`);
	}
	console.log('};');
}

// Colorspace conversion and coefficients from ISO IEC 23001-8:2018 (7.3) data
{
	const csc = {
		0: new ColorCoefficients(0.0, 0.0, 0.0, true).mat3_RGB_RGB().cFloat('AVCOL_SPC_RGB',
			'The identity matrix (RGB/XYZ); IEC 61966-2-1 sRGB; SMPTE-ST-428-1; ITU-R BT.709-5'),
		1: new ColorCoefficients(0.2126, 0.7152, 0.0722, false).mat3_YUV_RGB().cFloat('AVCOL_SPC_BT709',
			'ITU-R BT.709-5; ITU-R BT.1361; IEC 61966-2-1/4 sYCC/xvYCC709; SMPTE-RP-177:1993b'),
		2: { comment: 'UNSPECIFIED - Image characteristics are unknown or are determined by the application' },
		4: new ColorCoefficients(0.30, 0.59, 0.111, false).mat3_YUV_RGB().cFloat('AVCOL_SPC_FCC',
			'USFCCT-47:2003-73.682a'),
		5: new ColorCoefficients(0.299, 0.587, 0.114, false).mat3_YUV_RGB().cFloat('AVCOL_SPC_BT470BG',
			'ITU-R BT.470-6bg; ITU-R BT.601-6 625; ITU-R BT.1358 625; ITU-R BT.1700 625 PAL/SECAM; IEC 61966-2-4 xvYCC601'),
		6: new ColorCoefficients(0.299, 0.587, 0.114, false).mat3_YUV_RGB().cFloat('AVCOL_SPC_SMPTE170M',
			'ITU-R BT.601-6 525; ITU-R BT.1358 525; ITU-R BT.1700 NTSC; SMPTE-170M:2004'),
		7: new ColorCoefficients(0.212, 0.701, 0.087, false).mat3_YUV_RGB().cFloat('AVCOL_SPC_SMPTE240M',
			'SMPTE-240M:1999'),
		8: new ColorCoefficients(0.0, 0.0, 0.0, true).mat3_RGB_RGB().cFloat('AVCOL_SPC_YCGCO',
			'ITU-T SG16; Dirac/VC-2 and H.264 FRext'),
		9: new ColorCoefficients(0.2627, 0.678, 0.0593, false).mat3_YUV_RGB().cFloat('AVCOL_SPC_BT2020_NCL',
			'ITU-R BT.2020 (non-constant luminance)'),
		10: new ColorCoefficients(0.2627, 0.678, 0.0593, false).mat3_YUV_RGB().cFloat('AVCOL_SPC_BT2020_CL',
			'ITU-R BT.2020 (constant luminance)'),
		11: new ColorCoefficients(0.0, 0.0, 0.0, true).mat3_RGB_RGB().cFloat('AVCOL_SPC_SMPTE2085',
			'SMPTE-ST-2085:2015'),
		// 12-14 are not part of ISO
		12: { comment: 'TODO: AVCOL_SPC_CHROMA_DERIVED_NCL defined in FFmpeg?' },
		13: { comment: 'TODO: AVCOL_SPC_CHROMA_DERIVED_CL defined in FFmpeg?' },
		14: { comment: 'TODO: AVCOL_SPC_ICTCP defined in FFmpeg?' },
	};
	console.log('// from ISO IEC 23001-8:2018 (7.3) data')
	console.log('const static QMap<int, QVector<GLfloat>> _csc{');
	const n = Object.keys(csc).reduce((ac, v) => Math.max(ac, v), 0);
	for(let i = 0; i <= n; i++) {
		const e = csc[i];
		if(e && e.mat)
			console.log(`\t// ${i} - ${e.comment}\n\t{ ${i}, QVector<GLfloat>{ ${e.mat.join(', ')} }},`);
		else
			console.log(`\t// ${i} - ${e ? e.comment : 'RESERVED - For future use by ISO/IEC'}`);
	}
	console.log('};\n');
}

// Footer
console.log('\n}');


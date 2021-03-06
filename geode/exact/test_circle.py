#!/usr/bin/env python

from __future__ import division
from geode import *
import sys

def random_circle_arcs(n,k):
  '''Generate n random circular arc polygons with k arcs per contour'''
  arcs = empty((n,k),dtype=CircleArc).view(recarray)
  arcs.x = random.randn(n,1,2)+.5*random.randn(n,k,2)
  arcs.q = random.uniform(-1.5,1.5,size=n*k).reshape(n,k)
  return Nested(arcs)

def draw_circle_arcs(arcs,n=100,label=False,full=False,dots=False,jitter=None):
  import pylab
  for p,poly in enumerate(arcs):
    X = poly['x']
    q = poly['q']
    Xn = roll(X,-1,axis=0)
    l = magnitudes(Xn-X)
    center = .5*(X+Xn)+.25*(1/q-q).reshape(-1,1)*rotate_left_90(Xn-X)
    if 0:
      print 'draw %d: x0 %s, x1 %s, center %s'%(0,X[0],Xn[0],center[0])
      radius = .25*l*(1/q+q)
      print 'radius = %s'%radius
      assert allclose(magnitudes(X-center),abs(radius))
    theta = array([2*pi]) if full else 4*atan(q)
    points = center.reshape(-1,1,2)+Rotation.from_angle(theta[:,None]*arange(n+1)/n)*(X-center).reshape(-1,1,2)
    if dots:
      pylab.plot(X[:,0],X[:,1],'.')
    if full:
      for i,pp in enumerate(points):
        pylab.plot(pp[:,0],pp[:,1],'--')
        pylab.plot(center[i,0],center[i,1],'+')
        if label:
          pylab.annotate(str(arcs.offsets[p]+i),center[i])
    else:
      if label:
        for i in xrange(len(poly)):
          pylab.annotate(str(arcs.offsets[p]+i),points[i,n//2])
      points = concatenate([points.reshape(-1,2),[points[-1,-1]]])
      if jitter is not None:
         points += jitter*random.uniform(-1,1,points.shape) # Jitter if you want to be able to differentiate concident points:
      pylab.plot(points[:,0],points[:,1])

def subplot_arcs(arcs,subplot_index=111,title=None,full=True,label=True,dots=True,jitter=None):
  import pylab
  ax = pylab.subplot(subplot_index)
  if title is not None:
    pylab.title(title)
  if full:
    draw_circle_arcs(arcs,full=True,label=label,jitter=jitter)
  draw_circle_arcs(arcs,label=label,dots=dots,jitter=jitter)
  ax.set_aspect('equal')

def test_circle_reverse():
  random.seed(1240405)
  for k in [2,3,4,10,100]:
    for i in range(10):
      arcs = random_circle_arcs(1 + (i%2),k)
      area_before = circle_arc_area(arcs)
      reverse_arcs(arcs)
      area_after = circle_arc_area(arcs)
      assert abs(area_before + area_after) < 1e-7

def test_circle_quantize():
  random_circle_quantize_test(12312) # Test quantization for complete circles
  random.seed(37130)
  arcs0 = random_circle_arcs(1,100)
  #arcs0 = random_circle_arcs(20,5)
  arcs1 = circle_arc_quantize_test(arcs0)
  assert all(arcs0.offsets==arcs1.offsets)
  ex = relative_error(arcs0.flat['x'],arcs1.flat['x'])
  i = argmax(abs(arcs0.flat['q']-arcs1.flat['q']))
  q0,q1 = arcs0.flat['q'][i],arcs1.flat['q'][i]
  eq = abs(q0-q1)
  comparison_str = 'ex = %g, eq = %g (%d: %g to %g)'%(ex,eq,i,q0,q1)
  print comparison_str
  show_results = False # Enable this if you want comparisons between expected and actual results
  if show_results and not (ex<1e-6 and eq<3e-5):
    plot_args = dict(full=False, label=True, dots=True)
    import pylab
    pylab.suptitle(comparison_str)
    subplot_arcs(arcs0, 121, "Before Quantization", **plot_args)
    subplot_arcs(arcs1, 122, "After Quantization", **plot_args)
    pylab.show()


  assert ex<1e-6 and eq<3e-5 #This threshold is pretty agressive and might not work for many seeds

def to_arcs(py_arcs):
  arrays = []
  for contour in py_arcs:
    arrays.append(asarray([((a[0][0],a[0][1]),a[1]) for a in contour], dtype=CircleArc))
  return Nested(arrays, dtype=CircleArc)

def test_circles():
  # Test quantization routine
  random.seed(37130)
  # We don't know how to write a unit test, so use a regression test instead.  Each entry is indexed by (k,n,i):
  known = {(3,1,0):[]
          ,(3,1,1):[[[[-3.010773,0.755762],-0.336175],[[-2.858849,1.309345],0.589384],[[-2.848044,0.356576],0.698345]]]
          ,(3,1,2):[[[[-2.724896,-0.718846],-0.034550],[[-1.800262,-0.054708],1.110004]]]
          ,(3,1,3):[[[[-0.303861,1.024886],0.142447],[[0.656238,0.432930],-0.216057],[[0.573075,0.905945],0.297362]]]
          ,(3,1,4):[[[[-0.522519,-0.584078],-0.000456],[[0.181312,-1.825772],0.333828],[[0.316874,-0.423081],1.205249]]]
          ,(3,1,5):[[[[-2.301922,1.591784],0.590868],[[-2.248770,0.515245],-0.404141]]]
          ,(3,1,6):[[[[1.991475,-0.087749],0.058138],[[2.252511,-0.129968],0.079171]]]
          ,(3,1,7):[[[[1.686977,0.901313],-0.825506],[[2.173907,0.323295],-0.889160],[[2.394244,-0.216802],1.083369]]]
          ,(3,1,8):[[[[0.826107,-0.051135],-0.001554],[[0.828592,-0.052081],-0.000613],[[0.829624,-0.051610],-0.010869]]]
          ,(3,1,9):[]
          ,(3,2,0):[]
          ,(3,2,1):[[[[-2.216575,0.262133],0.840664],[[-1.715437,0.378521],-0.836815],[[-1.520104,1.476167],1.110229]],[[[0.733133,0.198975],0.201225],[[1.350660,0.190027],1.056223],[[0.828611,0.498085],-0.799089]]]
          ,(3,2,2):[[[[0.426560,0.796812],-0.015221],[[0.438715,0.801127],-0.008692],[[0.427843,0.808990],0.002491]]]
          ,(3,2,3):[[[[-1.189061,0.319839],-0.024296],[[-1.156225,0.315736],-0.008650],[[-1.155983,0.335169],0.046473]],[[[-1.149088,-0.354165],0.676161],[[-0.874696,-0.170949],-0.014745],[[-0.901609,-0.153892],-0.247980]],[[[-0.979996,0.312666],0.037660],[[-0.977415,0.224464],-0.105190],[[-0.898515,0.106663],0.308622]]]
          ,(3,2,4):[[[[-1.724795,0.024468],-0.061043],[[-1.619382,0.251626],-0.019812],[[-1.664982,0.292379],-0.095631],[[-1.716633,0.262827],0.098298]],[[[-1.178215,-0.486595],0.548680],[[-0.543385,0.328433],0.000952],[[-0.553205,0.331992],-0.225385],[[-1.164077,0.076010],-0.137576]]]
          ,(3,3,0):[[[[-2.666048,-1.426976],0.383100],[[-2.301528,-1.981148],0.693661],[[-1.673099,-2.802806],0.925914]],[[[1.887855,-2.119635],0.141599],[[3.836737,-2.084426],1.436873],[[2.200843,-1.110354],0.453311]]]
          ,(3,3,1):'boring'
          ,(3,3,2):[[[[-0.510294,0.704167],-0.028318],[[-0.093683,0.411961],0.901914],[[-0.136030,-0.807826],0.161745],[[0.439782,-0.251437],0.009192],[[0.155160,0.013840],0.068953],[[0.272982,0.088631],-0.016613],[[0.478865,-0.127765],0.072894],[[0.443719,-0.243846],0.022231],[[0.490841,-0.141275],-0.040178],[[0.923357,-0.718623],0.338586],[[1.273104,-0.350055],-0.306153],[[0.492852,-0.136300],0.087512],[[0.583693,0.295422],-0.189865],[[0.344185,0.162598],1.105698]],[[[-0.040488,0.369307],-0.016236],[[0.179752,0.177727],-0.203408]],[[[0.442339,-0.253911],0.480081],[[0.897114,-0.721975],0.015420]]]
          ,(3,3,3):[[[[-1.185738,-0.402147],0.126455],[[-0.880949,-1.409412],-0.123348],[[-0.850696,-0.873056],0.063155],[[-0.718194,-1.066745],1.362333],[[0.381933,0.324818],1.294009],[[0.290114,0.431820],0.512688],[[-0.953336,-0.432871],-0.054159]],[[[0.496306,0.333617],0.030982],[[0.994209,0.177878],0.113508],[[1.353632,0.193640],0.958044]]]
          ,(3,10,0):[[[[-1.453289,-2.809749],0.667486],[[-0.893038,-2.812135],1.490685],[[-1.150855,-2.549945],1.322488]],[[[-0.429733,-0.417243],0.109782],[[-0.409798,-0.636888],-0.092850],[[-0.281360,-0.786052],0.092081],[[-0.407264,-0.644607],0.147577],[[-0.237065,-0.883907],-0.214415],[[-0.279956,-1.320125],0.220890],[[-0.234773,-0.885674],0.136238],[[0.017467,-0.987308],-0.022269],[[0.588381,-0.410025],0.065205],[[-0.208725,-0.386703],-0.118615],[[-0.082548,-0.127479],0.005456],[[-0.237814,-0.081107],0.186024],[[-0.426253,-0.394363],-0.190440],[[-0.253192,-0.391536],0.014528]],[[[-0.213546,-0.410876],-0.481085],[[-0.167188,-0.775884],-0.152764]],[[[-0.007673,0.775326],0.176070],[[0.613992,0.413336],-0.089050],[[0.658953,0.505964],0.030758],[[0.680313,0.403523],0.060150],[[0.932777,0.404811],-0.022780],[[0.946646,0.441288],0.037220],[[0.950331,0.407172],0.137119],[[1.469238,0.635559],-0.123964],[[1.337842,1.336018],-0.159037],[[1.592977,0.755306],0.409897],[[1.507176,2.231095],1.039402],[[0.734075,2.406343],0.983069]],[[[0.223366,-1.242424],-0.235979],[[0.559076,-1.256569],0.859877],[[0.832823,-1.400939],0.379925]],[[[0.657113,-1.107158],0.214171],[[1.088048,-1.123091],1.005347]],[[[0.856394,-0.109806],-0.474681],[[1.257573,-1.012384],0.696716]],[[[1.494908,-0.128537],0.271431],[[1.717105,0.067760],-0.178489]]]
          ,(3,10,1):[[[[-2.503355,0.135105],-0.235193],[[-2.262554,-0.052479],1.248027],[[-2.484257,0.160738],0.000804]],[[[-2.058643,-0.230671],-0.319316],[[-1.559567,0.037099],-0.347047],[[-1.520080,0.972124],-0.127287],[[-0.884079,0.884983],-0.582577],[[-0.773175,1.214582],-0.071359],[[-0.573586,1.135969],-0.110738],[[-0.538833,1.055704],0.033427],[[-0.551665,1.122991],-0.154137],[[-0.253933,0.779053],-0.018736],[[-0.251264,0.721719],0.186409],[[0.102279,0.980313],0.222817],[[0.212778,0.686069],-0.055145],[[-0.251409,0.698373],-0.572340],[[-1.404591,0.053789],0.068171],[[-1.448225,0.009452],-0.216132],[[-1.163233,-0.276318],0.185753],[[-0.399292,-0.271834],-0.035356],[[-0.375776,-0.228709],-0.082833],[[1.403343e-05,0.003181],0.167021],[[0.273520,0.638108],0.483418],[[0.821076,0.829935],-0.066828],[[0.276247,0.692359],0.113827],[[0.180329,1.160739],0.011219],[[0.186372,1.187336],0.012633],[[0.174799,1.172644],0.395583],[[-1.178649,1.707578],0.434007]],[[[0.171906,-1.799055],0.264830],[[0.957447,-1.409122],-0.242764]],[[[0.563410,-0.477915],0.194847],[[1.259138,-0.127758],-0.183250],[[1.807019,-0.905868],0.396841],[[1.437020,0.158107],0.364968],[[1.034103,1.432217],0.148412],[[1.197739,0.255950],-0.006023],[[1.198297,0.238953],0.199504],[[0.583267,0.100560],-0.109097],[[1.140365,-0.056058],-0.271876]],[[[1.854054,0.808627],0.251675],[[1.945366,0.092593],-0.350994],[[2.320178,0.071094],0.115192]]]
          ,(3,10,2):[[[[-1.850146,2.031636],1.193297],[[-0.984657,1.698693],1.014384],[[-1.128600,1.713286],0.260469]],[[[-1.615670,0.262574],-0.144464],[[-1.466613,-0.348527],-0.121168],[[-1.356335,-0.012708],1.464144]],[[[-0.905989,1.906895],-0.162077],[[-0.764319,1.916432],0.020880],[[-0.785940,2.080328],-0.126287]],[[[-0.021940,0.793038],0.346693],[[0.391923,0.269867],0.038725],[[0.357187,0.077516],0.098515],[[0.564597,0.257287],0.130665],[[0.824858,0.351844],-0.048783],[[0.841263,0.228062],-0.126355],[[0.532853,0.027745],0.198267],[[0.775957,-0.076660],-0.292794],[[0.184168,-0.435120],1.496861],[[0.199749,-0.990425],0.447133],[[1.049210,0.050447],0.404356],[[1.009394,0.530790],-0.062213],[[0.938907,0.361636],0.018593],[[0.903196,0.418342],0.059189],[[0.980460,0.519300],-0.038557],[[0.984967,0.550321],0.155798],[[0.784737,0.616728],0.003545],[[0.773293,0.636896],-0.023078],[[0.736807,0.601684],0.009748],[[0.709403,0.623688],0.040277],[[0.710757,0.736803],0.003148],[[0.699102,0.753780],-0.007487],[[0.656040,0.773481],0.017096],[[0.601999,0.706070],-0.021003],[[0.642533,0.670598],0.014970],[[0.596151,0.698178],0.051963],[[0.463728,0.472149],-0.046316],[[0.362389,0.464862],0.037989],[[0.192825,0.810718],0.059983]],[[[-0.021763,0.807847],-0.077177],[[0.171174,0.846831],0.023279],[[0.038768,1.042495],0.113888]],[[[0.703049,0.574277],0.009093],[[0.706748,0.599587],-0.006917],[[0.717442,0.585423],-0.008281]],[[[1.031037,0.640434],-0.013174],[[1.038349,0.648279],-0.001011],[[1.032066,0.649494],-0.003083]]]
          ,(3,10,3):[[[[-2.688785,-1.212539],0.769230],[[-1.080329,-1.143351],-0.203676],[[-1.388706,-0.914265],-0.008203],[[-1.174639,-0.834228],-0.019157],[[-1.203638,-0.781211],0.146600],[[-1.424285,-0.839841],-0.258863],[[-1.355535,-0.368259],0.053953],[[-1.279858,-0.530940],-0.078995],[[-1.272768,-0.283315],-0.016662],[[-1.246601,-0.263668],-0.079393],[[-1.188952,-0.189290],-0.058420],[[-1.110709,0.066915],-0.011862],[[-1.086477,0.095440],0.002787],[[-1.093293,0.104767],-0.197982],[[-0.440406,0.689701],-0.009223],[[-0.449116,0.725582],0.309172],[[-1.292900,0.308640],0.918747]],[[[-0.729724,1.978493],0.470730],[[0.183745,2.094192],-0.207443]],[[[1.205548,1.853441],0.540626],[[2.324679,1.474149],0.316197],[[1.687294,1.499578],0.745920]]]
          ,(3,10,4):[[[[-1.713445,-1.244464],0.034561],[[-1.599586,-1.287285],0.081800],[[-1.306104,-1.085377],-0.227450],[[-1.512949,-0.621891],0.781910]],[[[-1.441501,1.437139],0.431596],[[-0.731085,1.545868],-0.191798],[[-1.250495,1.640124],-0.818925]],[[[-1.093375,-0.800970],0.235370],[[-1.076625,0.176586],0.400043]],[[[-0.802951,0.619093],-0.206489],[[-0.157361,-0.008610],-0.110921],[[-0.309586,-0.120498],0.067734],[[-0.108911,-0.152166],-0.105902],[[-0.086171,-0.627787],-0.037552],[[0.083356,-0.591277],0.045093],[[0.121198,-0.469432],0.629025],[[0.254361,-0.067518],0.227726],[[0.645948,0.454210],0.190258],[[1.166980,0.824583],1.155420],[[1.355540,1.073012],0.196773],[[0.615545,0.856931],0.009870],[[0.604889,0.884693],0.121660],[[0.107467,0.510668],0.501837]],[[[-0.296379,-1.164791],0.108890],[[-0.142394,-1.395791],0.138141],[[-0.295848,-1.160402],0.001700]],[[[-0.168680,-0.903918],0.115345],[[0.047980,-0.664735],0.086158],[[-0.128552,-0.799981],-0.024519]],[[[1.467030,-0.182280],0.285611],[[1.978747,-1.307214],-0.252715],[[1.872244,-0.268930],-0.282824]]]
          ,(3,10,5):[[[[-3.060150,-0.355061],0.224263],[[-2.431354,-0.431142],0.460216],[[-1.440838,-0.498447],1.370885]],[[[0.020557,0.738515],-0.019147],[[0.131563,0.767062],-0.045649],[[0.182079,0.960629],-0.050604],[[0.274621,1.016848],-0.076444],[[0.271610,1.154057],0.482941]],[[[0.416269,1.119232],-0.034160],[[0.433523,1.064196],-0.368116],[[1.018625,0.688291],-0.000390],[[1.020787,0.687399],1.023208]],[[[0.957602,0.711869],-0.001475],[[0.965901,0.708841],0.013196],[[0.992423,0.735853],-0.023427]],[[[2.197525,1.038404],-0.056303],[[2.274004,0.763559],0.147383]]]
          }
  def arc_error(correct,arcs):
    e = 0
    for cs,xs in zip(correct,arcs):
      for c,x in zip(cs,xs):
        e = max(e,maxabs(c[0]-x['x']),abs(c[1]-x['q']))
    return e

  # Test CSG
  k = 3
  plot_args = dict(full=False, label=True, dots=True)
  for n in 1,2,3,10,40,100:
    for i in xrange({1:10,2:5,3:4,10:6,40:20,100:10}[n]):
      correct = known.get((k,n,i))
      if correct=='boring':
        continue
      print '(k,n,i) (%d,%d,%d)'%(k,n,i)
      random.seed(18183181+1000*k+10*n+i)
      arcs0 = canonicalize_circle_arcs(random_circle_arcs(n,k))
      circle_arc_quantize_test(arcs0);
      if (k,n,i)==None: # Enable to visualize before union
        print
        print 'arcs0 = %s'%compact_str(arcs0)
        import pylab
        pylab.suptitle('k %d, n %d, i %d'%(k,n,i))
        subplot_arcs(arcs0,**plot_args)
        pylab.show()
      arcs1 = canonicalize_circle_arcs(circle_arc_union(arcs0))
      error = 0 if n>=40 else inf if correct is None else arc_error(correct,arcs1)
      if error>2e-5:
        print 'error = %f' % error
        print 'expected area = %f' % circle_arc_area(to_arcs(correct))
        print 'result area = %f' % circle_arc_area(arcs1)
        print 'arcs0 = %s'%compact_str(arcs0)
        print '\narcs1 = %s'%compact_str(arcs1)
        print '\ncorrect = %s'%compact_str(correct)
        if 0: # Enable this if you want comparisons between expected and actual results
          import pylab
          pylab.suptitle('k %d, n %d, i %d, error %g'%(k,n,i,error))
          subplot_arcs(arcs0, 121, "Input to union", **plot_args)
          subplot_arcs(arcs1, 122, "Output of union", **plot_args)
          pylab.figure()
          subplot_arcs(arcs0, 121, "Before Quantization", **plot_args)
          subplot_arcs(circle_arc_quantize_test(arcs0), 122, "After Quantization", **plot_args)
          pylab.figure()
          subplot_arcs(to_arcs(correct), 121, "Expected", **plot_args)
          subplot_arcs(arcs1, 122, "Returned", **plot_args)
          pylab.show()
        assert False
      # Check extremely degenerate situations
      if n==40 and i<2:
        area = circle_arc_area(arcs1)
        assert allclose(area,circle_arc_area(circle_arc_union(arcs1,arcs1)))
        assert allclose(area,circle_arc_area(circle_arc_intersection(arcs1,arcs1)))

def test_single_circle(show_results=False):
  seed = 151193
  max_count = 10
  for count in range(max_count):
    num_trials = 1 if count == 0 else 10
    for trial in range(num_trials):
      input_arcs, union_arcs, overlap_arcs = single_circle_handling_test(seed + trial*max_count + count, count)
      if show_results:
        import pylab
        plot_args = dict(full=False, label=True, dots=True)
        pylab.suptitle('seed %d, count %d'%(seed, count))
        subplot_arcs(input_arcs, 121, "Input arcs", **plot_args)
        subplot_arcs(union_arcs, 122, "Output of union", **plot_args)
        pylab.figure()
        subplot_arcs(input_arcs, 121, "Input arcs", **plot_args)
        subplot_arcs(overlap_arcs, 122, "Output of overlaps", **plot_args)
        pylab.show()


def test_offsets():
  random.seed(441424)
  arcs0 = circle_arc_union(random_circle_arcs(10,10))

  print "Offsetting arcs"
  arcs1 = offset_arcs(arcs0, 0.1)
  assert circle_arc_area(arcs1) > circle_arc_area(arcs0)

  print "Offsetting arcs with shells"
  shells = offset_shells(arcs0, 0.2, 10)
  # Check that we have monatonically increasing area
  prev_area, prev_arcs = 0, []
  for arcs in [arcs0, arcs1] + shells:
    area = circle_arc_area(arcs)

    if not area > prev_area:
      error = "Positive offset caused decrease in area from %g to %g" % (prev_area, area)
      print error
      if 0:
        import pylab
        pylab.suptitle(error)
        subplot_arcs(prev_arcs, 121, "Previous shell", full=False)
        subplot_arcs(arcs, 122, "After offset", full=False)
        pylab.show()
      assert False
    prev_area, prev_arcs = area, arcs

  print "Offsetting of open arcs"
  arcs4 = offset_open_arcs(arcs0, 0.001) # Mostly this just ensures we don't hit any asserts
  assert circle_arc_area(arcs4) > 0 # We should at least have a positive area

def test_negative_offsets(seed=7056389):
  print "Testing negative offset"
  random.seed(seed)
  d = 0.4
  # Offset inward then outward would normally erode sharp features, but we can use a positive offset to generate a shape with no sharp features
  arcs0 = offset_arcs(random_circle_arcs(10,10), d*1.5) # Generate random arcs and ensure features big enough to not disappear if we inset/offset again
  inset = offset_arcs(arcs0, -d)
  reset = offset_arcs(inset, d)
  arcs0_area = circle_arc_area(arcs0)
  inset_area = circle_arc_area(inset)
  reset_area = circle_arc_area(reset)
  assert inset_area < arcs0_area # Offset by negative amount should reduce area
  area_error = abs(arcs0_area - reset_area)
  assert area_error < 2e-6
  # xor input arcs and result after inset/offset to get difference
  delta = split_arcs_by_parity(Nested.concatenate(arcs0,reset))
  # We expect thin features around edges of input arcs, but a small negative offset should erase everything
  squeezed_delta = offset_arcs(delta,-1e-6)
  assert len(squeezed_delta) == 0

  # Check that a large negative offset leaves nothing
  empty_arcs = offset_arcs(random_circle_arcs(10,10), -100.)
  assert len(empty_arcs) == 0

if __name__=='__main__':
  test_offsets()
  test_negative_offsets()
  test_circle_quantize()
  test_single_circle()
  test_circles()

#!/usr/bin/env ruby
#
# alg3d.rb : an any-dimensional vector and 3D spinor (quaternion)
#
# Copyright (c) 2001 Issac Trotts <itrotts at idolminds dot com>
# Under the Ruby license
# 

module GLIT # alg3d is part of the GL Interaction Tools library

    class Vec  # any-dimensional vector class
        attr_accessor :a # array of elements

        #ZERO_VEC = GLIT::Vec.new([0.0, 0.0, 0.0])

        def initialize(a=[0.0, 0.0, 0.0])
            if not a.respond_to?('[]') 
                raise ArgumentError
            end
            @a = a; 
        end
        def [](i)
            @a[i]
        end
        def []=(i, x)
            @a[i] = x
        end
        def abs
            Math.sqrt( self.dot(self) )
        end
        def abs2
            self.dot(self)
        end
        def copy
            Vec.new(@a[0..-1])
        end
        def normalized
            self.copy/self.abs
        end
        def normalize!
            mag = self.abs
            0.upto(@a.size-1) { |i| @a[i] /= mag }
        end
        def dot(v)
            (self.size == v.size) or raise ArgumentError 
            s = 0.0
            0.upto(@a.size-1) { |i| s+= @a[i]*v[i] }
            s
        end
        def gp(v) # clifford / geometric product : returns Quat
            (self.size == v.size) or raise ArgumentError 
            Quat.sv2q(self.dot(v), self.cross(v))
        end
        def cross(v)
            (self.size == 3 and v.size == 3) or raise ArgumentError
            Vec.new([@a[1]*v[2]-@a[2]*v[1],
                     @a[2]*v[0]-@a[0]*v[2],
                     @a[0]*v[1]-@a[1]*v[0]])
        end
        #def collect
            #Vec.new( @a.collect ) # ?
        #end
        #def collect2(v)
            #Vec.new( (0 .. @a.size-1).collect { |i| yield @a[i], v[i] } )
        #end
        #alias map  collect
        #alias map2 collect2
        def +(v) # v = vector
            (self.size == v.size) or raise ArgumentError 
            r = self.copy
            0.upto(@a.size-1) { |i| r[i] += v[i] }
            r
        end
        def -(v)
            (self.size == v.size) or raise ArgumentError 
            r = self.copy
            0.upto(@a.size-1) { |i| r[i] -= v[i] }
            r
        end
        def *(s) # s = scalar
            s.kind_of?(Float) or raise ArgumentError
            r = self.copy
            0.upto(@a.size-1) { |i| r[i] *= s }
            r
            #self.map { |x| x*s }
        end
        def /(s)
            s.kind_of?(Float) or raise ArgumentError
            r = self.copy
            0.upto(@a.size-1) { |i| r[i] /= s }
            r
            #self.map { |x| x/s }
        end
        def size
            @a.size
        end
    end
    
    def vmin(vec1, vec2)
      v = vec1.copy
      (0...v.size).each do |i|
        v[i] = vec2[i]   if vec1[i] > vec2[i]  # already initialized with vec1
      end
      v
    end
    
    def vmax(vec1, vec2)
      v = vec1.copy
      (0...v.size).each do |i|
        v[i] = vec2[i]   if vec1[i] < vec2[i]  # already initialized with vec1
      end
      v
    end

    include Math

   
    class Quat   # Quaternion (a.k.a. 3D Spinor) s+B = s+Iv

        attr_reader :a     # array of elements

        def initialize(a=[0.0, 0.0, 0.0, 1.0])
            if a.respond_to?('[]')  # all 4 elts of quat specified by 4D vector
                if a.size == 4
                    @a = a[0..3]
                elsif a.size == 3   # quat from 3D vector
                    @a = a[0..2]+[0.0]
                else
                    raise ArgumentError
                end
            elsif a.kind_of?(Float) or a.kind_of?(Fixnum) # quat from scalar
                @a = [0.0, 0.0, 0.0, a]
            end 
        end
        def copy
            Quat.new( @a[0..-1] )
        end
        ##
        # construct quat rotating vector f to vector t (up to scale)
        def Quat.vv2q(f, t) 
            (f.respond_to?('[]') and f.size == 3 and 
             t.respond_to?('[]') and t.size == 3) or raise ArgumentError
            f = f.normalized
            t = t.normalized
            f.gp(f+t)
            ft = f.gp(t)
            Quat.sv2q(1.0+ft[3], ft[0..2]) / (f+t).abs
        end
        def Quat.rotation(f, t)
            Quat.vv2q(f, t)
        end
        def Quat.sv2q(s, v)   # scalar and vector --> new quat
            (s.kind_of?(Float) and v.respond_to?('[]') and v.size == 3) or 
                raise ArgumentError
            Quat.new(v[0..2]+[s]) # array-concatenation, not vec addition
        end
        def Quat.rpy2q(roll, pitch, yaw) # pilferred from the FoX GUI lib
            r=0.5*roll;
            p=0.5*pitch;
            y=0.5*yaw;
            sr=sin(r); cr=cos(r);
            sp=sin(p); cp=cos(p);
            sy=sin(y); cy=cos(y);
            Quat.new([
                sr*cp*cy-cr*sp*sy,
                cr*sp*cy+sr*cp*sy,
                cr*cp*sy-sr*sp*cy,
                cr*cp*cy+sr*sp*sy])
        end

        def exp # Quaternion exponential
            mag = self.v.abs 
            Math.exp(self.s) * Quat.sv2q(cos(mag), sin(mag)*self.v/mag)
        end

        # Rotation about axis ax by t_rads radians
        def Quat.axis_rotation(ax, t_rads) 
            if len=axis.abs
                a=sin(t/2.0)*ax/len;
                Quat.sv2q(Math.cos(t/2.0), a)
            else
                Quat.sv2q(1.0, [0.0]*3)
            end
        end


        def rotate(v)
            (self.inverse * Quat.new(v) * self ).v 
        end
        def abs2
            @a[0]*@a[0] + @a[1]*@a[1] + @a[2]*@a[2] + @a[3]*@a[3] 
        end
        def abs
            Math.sqrt(@a[0]*@a[0] + @a[1]*@a[1] + @a[2]*@a[2] + @a[3]*@a[3])
        end
        def normalize!
            mag = self.abs
            @a[0] /= mag
            @a[1] /= mag
            @a[2] /= mag
            @a[3] /= mag
        end
        def s       # scalar part
            @a[3]
        end
        def v       # dual of bivector part
            Vec.new(@a[0..2])
        end
        def [](i)
            @a[i]
        end
        def axis 
            vlen=self.v.abs
            vlen>0.0 ? self.v/vlen : Vec.new([0.0, 0.0, 0.0])
        end
        def angle_rads
            2.0*Math.atan2(self.v.abs, self.s) 
        end
        def *(o) # o = Float, Fixnum or Quat
            if o.kind_of?(Float) or o.kind_of?(Fixnum)
                Quat.new([ @a[0]*o, @a[1]*o, @a[2]*o, @a[3]*o ])
            elsif o.kind_of?(Quat)
                s = self.s*o.s - self.v.dot(o.v)
                v = self.s*o.v - self.v.cross(o.v) + self.v*o.s
                Quat.sv2q(s, v)
            else
                raise ArgumentError
            end
        end
        def /(s) # s = scalar (Float)
            if s.kind_of?(Float) or s.kind_of?(Fixnum)
                Quat.new([ @a[0]/s, @a[1]/s, @a[2]/s, @a[3]/s ])
            else
                raise ArgumentError, "s = #{s.inspect}"
            end
        end
        def +(q) # q = quat
            Quat.new([ @a[0]+q[0],  @a[1]+q[1],  @a[2]+q[2],  @a[3]+q[3] ])
        end
        def -(q)
            Quat.new([ @a[0]-q[0],  @a[1]-q[1],  @a[2]-q[2],  @a[3]-q[3] ])
        end
        def conj 
            Quat.new([ -@a[0], -@a[1], -@a[2], @a[3] ])
        end
        def inverse
            s = self.abs2
            Quat.new( [-@a[0]/s, -@a[1]/s, -@a[2]/s, @a[3]/s] )
        end
    end

end # module GLIT

# left-multiplication by floats
class Float
    alias mul *
    def *(o)
        if o.kind_of?(GLIT::Vec)
            o*self
        elsif o.kind_of?(GLIT::Quat)
            o*self
        else
            self.mul(o)
        end
    end
end

require 'runit/testcase'

class Alg3dtest < RUNIT::TestCase
    include GLIT

	def setup
        @N = 100        # number of times to run each random test 
        @eps = 1e-9     # error tolerance
	end
	
    def test_vector
        puts
        0.upto(@N) {
            u = Vec.new([rand(), rand(), rand()])
            v = Vec.new([rand(), rand(), rand()])
            w = u.cross(v)
            err = w.dot(u)
            assert(err < @eps)
            err = w.dot(v)
            assert(err < @eps)
            u.normalize!
            assert((1.0 - u.abs).abs < @eps)
        }
    end

	def test_rotation
        puts
        0.upto(@N) {
            u = Vec.new([rand(), rand(), rand()])
            v = GLIT::Vec.new([rand(), rand(), rand()])
            q = GLIT::Quat.rotation(u,v)
            #puts "q = #{q.inspect}"
            v2 = q.rotate(u)
            #puts "u = #{u.inspect}"
            #puts "v = #{v.inspect}"
            #puts "v2 = #{v2.inspect}"
            err = (v.normalized-v2.normalized).abs
            puts "err = #{err}"
            #puts "u.abs = #{u.abs}"
            #puts "v2.abs = #{v2.abs}"
            assert(err < @eps)
        }
	end

    def test_quat_inverse
        puts
        0.upto(@N) {
            q = GLIT::Quat.new([rand(), rand(), rand(), rand()])
            #puts "q*q.inverse = #{(q*q.inverse).inspect}"
            err = (q*q.inverse - Quat.new([0.0]*3 + [1.0])).abs
            puts "err = #{err}"
            assert(err < @eps)
        }
    end

end

#--- main program ----
if __FILE__ == $0
	require 'runit/cui/testrunner'
	RUNIT::CUI::TestRunner.run(Alg3dtest.suite)
end

		


